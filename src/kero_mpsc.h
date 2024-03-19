#ifndef KERO_MPSC_H
#define KERO_MPSC_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

#define KERO_STRUCT_TYPE_PIN(strt)                                             \
  strt(strt&&) = delete;                                                       \
  strt(const strt&) = delete;                                                  \
  auto operator=(strt&&) -> strt& = delete;                                    \
  auto operator=(const strt&) -> strt& = delete

#define KERO_STRUCT_TYPE_MOVE(strt)                                            \
  strt(strt&&) = default;                                                      \
  auto operator=(strt&&) -> strt& = default;                                   \
                                                                               \
  strt(const strt&) = delete;                                                  \
  auto operator=(const strt&) -> strt& = delete

#define KERO_STRUCT_TYPE_COPY(strt)                                            \
  strt(strt&&) = default;                                                      \
  strt(const strt&) = default;                                                 \
  auto operator=(strt&&) -> strt& = default;                                   \
  auto operator=(const strt&) -> strt& = default

namespace kero {

template <typename T>
concept MoveOnly =                           //
    std::is_object_v<T> &&                   //
    std::is_nothrow_destructible_v<T> &&     //
    std::is_constructible_v<T, T> &&         //
    !std::is_constructible_v<T, const T> &&  //
    !std::is_constructible_v<T, T&> &&       //
    !std::is_constructible_v<T, const T&> && //
    std::swappable<T> &&                     //
    std::convertible_to<T, T> &&             //
    std::convertible_to<T, const T> &&       //
    !std::convertible_to<T, T&> &&           //
    std::convertible_to<T, const T&> &&      //
    std::assignable_from<T&, T> &&           //
    !std::assignable_from<T&, const T> &&    //
    !std::assignable_from<T&, T&> &&         //
    !std::assignable_from<T&, const T&>;

namespace mpsc {

namespace impl {

template <typename T>
  requires MoveOnly<T>
class Queue {
public:
  class Builder {
  public:
    Builder() = default;
    ~Builder() = default;
    KERO_STRUCT_TYPE_PIN(Builder);

    auto Build() -> std::shared_ptr<Queue<T>> {
      return std::shared_ptr<Queue<T>>{new Queue<T>{}};
    }
  };

  ~Queue() = default;
  KERO_STRUCT_TYPE_PIN(Queue);

  auto Push(T&& item) -> void {
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

template <typename T>
  requires MoveOnly<T>
class Tx {
public:
  Tx(const std::shared_ptr<impl::Queue<T>>& queue) : queue_{queue} {}
  ~Tx() = default;
  KERO_STRUCT_TYPE_MOVE(Tx);

  auto Clone() const -> Tx<T> { return Tx<T>{queue_}; }
  auto Send(T&& item) const -> void { queue_->Push(std::move(item)); }

private:
  std::shared_ptr<impl::Queue<T>> queue_;
};

template <typename T>
  requires MoveOnly<T>
class Rx {
public:
  Rx(const std::shared_ptr<impl::Queue<T>>& queue) : queue_{queue} {}
  ~Rx() = default;
  KERO_STRUCT_TYPE_MOVE(Rx);

  auto Receive() const -> T { return queue_->Pop(); }

private:
  std::shared_ptr<impl::Queue<T>> queue_;
};

template <typename T>
  requires MoveOnly<T>
struct Channel {
  class Builder {
  public:
    Builder() = default;
    ~Builder() = default;
    KERO_STRUCT_TYPE_PIN(Builder);

    auto Build() -> Channel<T> {
      auto queue = typename impl::Queue<T>::Builder{}.Build();
      auto tx = Tx<T>{queue};
      auto rx = Rx<T>{queue};
      return Channel<T>{std::move(tx), std::move(rx)};
    }
  };

  Tx<T> tx;
  Rx<T> rx;

  ~Channel() = default;
  KERO_STRUCT_TYPE_MOVE(Channel);

private:
  Channel(Tx<T>&& tx, Rx<T>&& rx) : tx{std::move(tx)}, rx{std::move(rx)} {}
};

} // namespace mpsc
} // namespace kero

#endif // KERO_MPSC_H
