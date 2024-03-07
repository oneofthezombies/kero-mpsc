#ifndef KERO_MPSC_H
#define KERO_MPSC_H

#include <concepts>
#include <mutex>
#include <queue>
#include <utility>

namespace kero {

template <typename T>
concept Noncopyable =                         //
    std::is_object_v<T> &&                    //
    std::swappable<T> &&                      //
    std::constructible_from<T, T> &&          //
    !std::constructible_from<T, const T> &&   //
    !std::constructible_from<T, T &> &&       //
    !std::constructible_from<T, const T &> && //
    std::convertible_to<T, T> &&              //
    !std::convertible_to<T, const T> &&       //
    !std::convertible_to<T, T &> &&           //
    !std::convertible_to<T, const T &> &&     //
    std::assignable_from<T &, T> &&           //
    !std::assignable_from<T &, const T> &&    //
    !std::assignable_from<T &, T &> &&        //
    !std::assignable_from<T &, const T &>;

namespace mpsc {

namespace impl {

template <typename T>
  requires Noncopyable<T>
class Queue {
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
