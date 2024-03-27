#include "internal/channel.h"
#include <gtest/gtest.h>

struct Message {
  int id;
  std::string text;

  Message(int id, std::string&& text) noexcept
      : id(id), text(std::move(text)) {}

  Message(Message&&) noexcept = default;
  ~Message() noexcept = default;
  auto operator=(Message&&) noexcept -> Message& = default;

  Message(const Message&) = delete;
  Message& operator=(const Message&) = delete;
};

TEST(ChannelTest, Create) {
  auto [tx, rx] = kero::mpsc::Channel<Message>::Builder{}.Build();
}

TEST(ChannelTest, SendAndReceive) {
  auto [tx, rx] = kero::mpsc::Channel<Message>::Builder{}.Build();

  auto message = Message{1, "Hello, World!"};
  tx.Send(std::move(message));
  ASSERT_EQ(message.id, 1);
  ASSERT_EQ(message.text, "");

  auto popped = rx.Receive();
  ASSERT_EQ(popped.id, 1);
  ASSERT_EQ(popped.text, "Hello, World!");
}
