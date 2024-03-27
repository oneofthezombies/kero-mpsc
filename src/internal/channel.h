#ifndef KERO_MPSC_INTERNAL_CHANNEL_H
#define KERO_MPSC_INTERNAL_CHANNEL_H

#include "queue.h"
#include "rx.h"
#include "tx.h"

namespace kero {
namespace mpsc {

template <typename T>
  requires MoveOnly<T>
struct Channel {
  class Builder {
  public:
    Builder() noexcept = default;
    ~Builder() noexcept = default;

    Builder(Builder&&) = delete;
    Builder(const Builder&) = delete;
    auto operator=(Builder&&) -> Builder& = delete;
    auto operator=(const Builder&) -> Builder& = delete;

    auto Build() const noexcept -> Channel<T> {
      auto queue = typename Queue<T>::Builder{}.Build();
      auto tx = Tx<T>{queue};
      auto rx = Rx<T>{queue};
      return Channel<T>{std::move(tx), std::move(rx)};
    }
  };

  Tx<T> tx;
  Rx<T> rx;

  Channel(Channel&&) noexcept = default;
  ~Channel() noexcept = default;
  auto operator=(Channel&&) noexcept -> Channel& = default;

  Channel(const Channel&) = delete;
  auto operator=(const Channel&) -> Channel& = delete;

private:
  Channel(Tx<T>&& tx, Rx<T>&& rx) noexcept
      : tx{std::move(tx)}, rx{std::move(rx)} {}
};

} // namespace mpsc
} // namespace kero

#endif // KERO_MPSC_INTERNAL_CHANNEL_H