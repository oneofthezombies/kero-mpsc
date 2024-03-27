#ifndef KERO_MPSC_INTERNAL_QUEUE_H
#define KERO_MPSC_INTERNAL_QUEUE_H

#include "core.h" // IWYU pragma: keep
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>

namespace kero {
namespace mpsc {

template <typename T>
  requires MoveOnly<T>
class Queue {
public:
  class Builder {
  public:
    Builder() noexcept = default;
    ~Builder() noexcept = default;

    Builder(Builder&&) = delete;
    Builder(const Builder&) = delete;
    auto operator=(Builder&&) -> Builder& = delete;
    auto operator=(const Builder&) -> Builder& = delete;

    auto Build() const noexcept -> std::shared_ptr<Queue<T>> {
      return std::shared_ptr<Queue<T>>{new Queue<T>{}};
    }
  };

  ~Queue() noexcept = default;

  Queue(Queue&&) = delete;
  Queue(const Queue&) = delete;
  auto operator=(Queue&&) -> Queue& = delete;
  auto operator=(const Queue&) -> Queue& = delete;

  auto Push(T&& item) noexcept -> void {
    std::lock_guard<std::mutex> lock{mutex_};
    queue_.push(std::move(item));
    condition_variable_.notify_one();
  }

  auto Pop() noexcept -> T {
    std::unique_lock<std::mutex> lock{mutex_};
    condition_variable_.wait(lock, [this] { return !queue_.empty(); });
    auto item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  auto TryPop() noexcept -> std::optional<T> {
    std::lock_guard<std::mutex> lock{mutex_};
    if (queue_.empty()) {
      return std::nullopt;
    }
    auto item = std::move(queue_.front());
    queue_.pop();
    return item;
  }

  auto TryPopAll() noexcept -> std::queue<T> {
    std::lock_guard<std::mutex> lock{mutex_};
    auto queue = std::move(queue_);
    return queue;
  }

private:
  Queue() noexcept = default;

  std::queue<T> queue_{};
  std::mutex mutex_{};
  std::condition_variable condition_variable_{};
};

} // namespace mpsc
} // namespace kero

#endif // KERO_MPSC_INTERNAL_QUEUE_H