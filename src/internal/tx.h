#ifndef KERO_MPSC_INTERNAL_TX_H
#define KERO_MPSC_INTERNAL_TX_H

#include "queue.h"

namespace kero {
namespace mpsc {

template <typename T>
  requires MoveOnly<T>
class Tx {
public:
  Tx(const std::shared_ptr<Queue<T>>& queue) noexcept : queue_{queue} {}
  Tx(Tx&&) noexcept = default;
  ~Tx() noexcept = default;
  auto operator=(Tx&&) noexcept -> Tx& = default;

  Tx(const Tx&) = delete;
  auto operator=(const Tx&) -> Tx& = delete;

  auto Clone() const noexcept -> Tx<T> { return Tx<T>{queue_}; }
  auto Send(T&& item) const noexcept -> void { queue_->Push(std::move(item)); }

private:
  std::shared_ptr<Queue<T>> queue_;
};

} // namespace mpsc
} // namespace kero

#endif // KERO_MPSC_INTERNAL_TX_H