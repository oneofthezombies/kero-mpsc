#ifndef KERO_MPSC_INTERNAL_CORE_H
#define KERO_MPSC_INTERNAL_CORE_H

#include <concepts> // IWYU pragma: keep

namespace kero {
namespace mpsc {

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

} // namespace mpsc
} // namespace kero

#endif // KERO_MPSC_INTERNAL_CORE_H
