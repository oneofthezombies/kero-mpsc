#ifndef KERO_MPSC_INTERNAL_RX_H
#define KERO_MPSC_INTERNAL_RX_H

#include "queue.h"

namespace kero {
namespace mpsc {

template <typename T>
  requires MoveOnly<T>
class Rx {
public:
  Rx(const std::shared_ptr<Queue<T>>& queue) noexcept : queue_{queue} {}
  Rx(Rx&&) noexcept = default;
  ~Rx() noexcept = default;
  auto operator=(Rx&&) noexcept -> Rx& = default;

  Rx(const Rx&) = delete;
  auto operator=(const Rx&) -> Rx& = delete;

  auto Receive() const noexcept -> T { return queue_->Pop(); }
  auto TryReceive() const noexcept -> std::optional<T> {
    return queue_->TryPop();
  }
  auto TryReceiveAll() const noexcept -> std::queue<T> {
    return queue_->TryPopAll();
  }

private:
  std::shared_ptr<Queue<T>> queue_;
};

} // namespace mpsc
} // namespace kero

#endif // KERO_MPSC_INTERNAL_RX_H