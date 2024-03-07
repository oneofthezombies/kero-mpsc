#ifndef KERO_MPSC_H
#define KERO_MPSC_H

#include <mutex>
#include <queue>
#include <utility>

namespace kero {
namespace mpsc {

namespace impl {

template <typename T> class Queue {
public:
  class Builder {
  public:
    Builder() = default;
    ~Builder() = default;

    Builder(Builder &&) = delete;
    Builder(const Builder &) = delete;
    auto operator=(Builder &&) -> Builder & = delete;
    auto operator=(const Builder &) -> Builder & = delete;

    auto Build() -> std::shared_ptr<Queue<T>> {
      return std::shared_ptr<Queue<T>>{new Queue<T>{}};
    }
  };

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

  Queue(Queue &&) = delete;
  Queue(const Queue &) = delete;
  Queue &operator=(Queue &&) = delete;
  Queue &operator=(const Queue &) = delete;

  auto Push(T &&item) -> void {
    std::lock_guard<std::mutex> lock{mutex_};
    queue_.push(std::move(item));
    condition_variable_.notify_one();
  }

  auto Pop() -> T {
    std::unique_lock<std::mutex> lock{mutex_};
    condition_variable_.wait(lock, [this] { return !queue_.empty(); });
    auto item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

private:
  Queue() = default;

  std::queue<T> queue_{};
  std::mutex mutex_{};
  std::condition_variable condition_variable_{};
};

} // namespace impl

template <typename T> class Tx {
public:
  Tx(const std::shared_ptr<impl::Queue<T>> &queue) : queue_{queue} {}

  Tx(Tx &&) = default;
  ~Tx() = default;
  Tx &operator=(Tx &&) = default;

  Tx(const Tx &) = delete;
  Tx &operator=(const Tx &) = delete;

  auto Clone() const -> Tx<T> { return Tx<T>{queue_}; }

  auto Send(T &&item) const -> void { queue_->Push(std::move(item)); }

private:
  std::shared_ptr<impl::Queue<T>> queue_;
};

template <typename T> class Rx {
public:
  Rx(const std::shared_ptr<impl::Queue<T>> &queue) : queue_{queue} {}

  Rx(Rx &&) = default;
  ~Rx() = default;
  Rx &operator=(Rx &&) = default;

  Rx(const Rx &) = delete;
  Rx &operator=(const Rx &) = delete;

  auto Receive() const -> T { return queue_->Pop(); }

private:
  std::shared_ptr<impl::Queue<T>> queue_;
};

template <typename T> struct Channel {
  class Builder {
  public:
    Builder() = default;
    ~Builder() = default;

    Builder(Builder &&) = delete;
    Builder(const Builder &) = delete;
    auto operator=(Builder &&) -> Builder & = delete;
    auto operator=(const Builder &) -> Builder & = delete;

    auto Build() -> Channel<T> {
      auto queue = typename impl::Queue<T>::Builder{}.Build();
      auto tx = Tx<T>{queue};
      auto rx = Rx<T>{queue};
      return Channel<T>{std::move(tx), std::move(rx)};
    }
  };

  Tx<T> tx;
  Rx<T> rx;

  Channel(Channel &&) = default;
  ~Channel() = default;
  auto operator=(Channel &&) -> Channel & = default;

  Channel(const Channel &) = delete;
  auto operator=(const Channel &) -> Channel & = delete;

private:
  Channel(Tx<T> &&tx, Rx<T> &&rx) : tx{std::move(tx)}, rx{std::move(rx)} {}
};

} // namespace mpsc
} // namespace kero

#endif // KERO_MPSC_H
