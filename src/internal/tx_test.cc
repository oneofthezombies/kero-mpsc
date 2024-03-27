#include "internal/tx.h"
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

TEST(TxTest, Create) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);
}

TEST(TxTest, Move) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto tx2 = std::move(tx);
  ASSERT_EQ(queue.use_count(), 2);
}

TEST(TxTest, Clone) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto tx2 = tx.Clone();
  ASSERT_EQ(queue.use_count(), 3);
}

TEST(TxTest, Send) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto message = Message{1, "Hello, World!"};
  tx.Send(std::move(message));
  ASSERT_EQ(message.id, 1);
  ASSERT_EQ(message.text, "");
}
