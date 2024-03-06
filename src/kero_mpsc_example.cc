#include "kero_mpsc.h"
#include <cassert>
#include <thread>

struct MyMessage {
  int id{};
  std::string text{};

  // ensure that MyMessage is move constructible and move assignable.
  // Kero MPSC is designed to work with move only types.
  // If your type is not move only, you can use std::unique_ptr to make it move
  // only.
  MyMessage(MyMessage &&) = default;
  MyMessage &operator=(MyMessage &&) = default;
  ~MyMessage() = default;

  MyMessage(const MyMessage &) = delete;
  MyMessage &operator=(const MyMessage &) = delete;
};

auto main() -> int {
  // create message passing channel
  auto [tx, rx] = kero::mpsc::mpsc<MyMessage>();

  // create thread to send message
  std::thread sender([tx = std::move(tx)] {
    auto message = MyMessage{1, "Hello, World!"};
    tx->send(std::move(message));
  });

  // tx is moved from, so it is now empty
  assert(!tx);
  // auto tx2 = tx; // error: use of deleted function

  // receive message
  auto message = rx->receive();
  assert(message.id == 1);
  assert(message.text == "Hello, World!");

  // wait for sender thread to finish
  sender.join();
  return 0;
}
