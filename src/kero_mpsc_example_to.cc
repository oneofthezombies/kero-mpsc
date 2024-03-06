#include "kero_mpsc.h"
#include <cassert>
#include <thread>

struct MyMessage {
  int id{};
  std::string text{};

  MyMessage(const MyMessage &) = default;
  MyMessage &operator=(const MyMessage &) = default;
  ~MyMessage() = default;
};

// MyMessage is not move only, so we use std::unique_ptr to make it move only.
using MyTransferObject = std::unique_ptr<MyMessage>;

auto main() -> int {
  // create message passing channel
  auto [tx, rx] = kero::mpsc::mpsc<MyTransferObject>();

  // create thread to send message
  std::thread sender([tx = std::move(tx)] {
    MyTransferObject to =
        std::make_unique<MyMessage>(MyMessage{1, "Hello, World!"});
    tx->send(std::move(to));
  });

  // tx is moved from, so it is now empty
  assert(!tx);
  // auto tx2 = tx; // error: use of deleted function

  // receive message
  auto message = rx->receive();
  assert(message->id == 1);
  assert(message->text == "Hello, World!");

  // wait for sender thread to finish
  sender.join();
  return 0;
}
