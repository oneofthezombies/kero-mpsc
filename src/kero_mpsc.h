#ifndef KERO_MPSC_H
#define KERO_MPSC_H

#include <mutex>
#include <queue>
#include <utility>

namespace kero {
namespace mpsc {

namespace impl {
template <typename T> struct Queue {
  // T must be move constructible and move assignable but not copy
  // constructible or copy assignable.
  static_assert(std::is_move_constructible<T>::value,
                "T must be move constructible");
  static_assert(std::is_move_assignable<T>::value, "T must be move assignable");
  static_assert(!std::is_copy_constructible<T>::value,
                "T must not be copy constructible");
  static_assert(!std::is_copy_assignable<T>::value,
                "T must not be copy assignable");

  ~Queue() = default;

  // No copy or move operations.
  Queue(const Queue &) = delete;
  Queue(Queue &&) = delete;
  Queue &operator=(const Queue &) = delete;
  Queue &operator=(Queue &&) = delete;

  auto push(T &&item) -> void {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(item));
    condition_variable_.notify_one();
  }

  auto pop() -> T {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_variable_.wait(lock, [this] { return !queue_.empty(); });
    auto item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  static auto create() -> std::shared_ptr<Queue<T>> {
    return std::shared_ptr<Queue<T>>(new Queue<T>{});
  }

private:
  // Private constructor to force use of create() method.
  Queue() = default;

  std::queue<T> queue_{};
  std::mutex mutex_{};
  std::condition_variable condition_variable_{};
};

} // namespace impl

template <typename T> struct Tx {
  Tx(const std::shared_ptr<impl::Queue<T>> &queue) : queue_{queue} {}
  Tx(Tx &&) = default;
  Tx &operator=(Tx &&) = default;
  ~Tx() = default;

  Tx(const Tx &) = delete;
  Tx &operator=(const Tx &) = delete;

  auto clone() const -> Tx<T> { return Tx<T>{queue_}; }

  auto send(T &&item) const -> void { queue_->push(std::move(item)); }

private:
  std::shared_ptr<impl::Queue<T>> queue_;
};

template <typename T> struct Rx {
  Rx(const std::shared_ptr<impl::Queue<T>> &queue) : queue_{queue} {}
  Rx(Rx &&) = default;
  Rx &operator=(Rx &&) = default;
  ~Rx() = default;

  Rx(const Rx &) = delete;
  Rx &operator=(const Rx &) = delete;

  auto receive() const -> T { return queue_->pop(); }

private:
  std::shared_ptr<impl::Queue<T>> queue_;
};

template <typename T> auto channel() -> std::pair<Tx<T>, Rx<T>> {
  auto queue = impl::Queue<T>::create();
  auto tx = Tx<T>{queue};
  auto rx = Rx<T>{queue};
  return std::make_pair(std::move(tx), std::move(rx));
}

} // namespace mpsc
} // namespace kero

#endif // KERO_MPSC_H
