#include "kero_mpsc.h"
#include <gtest/gtest.h>

struct Message {
  int id;
  std::string text;

  Message(int id, std::string &&text) : id(id), text(std::move(text)) {}

  Message(Message &&) = default;
  ~Message() = default;
  Message &operator=(Message &&) = default;

  Message(const Message &) = delete;
  Message &operator=(const Message &) = delete;
};

struct CopyableMessage {
  int id;
  std::string text;

  CopyableMessage(int id, std::string &&text) : id(id), text(std::move(text)) {}

  CopyableMessage(const CopyableMessage &) = default;
  ~CopyableMessage() = default;
  CopyableMessage &operator=(const CopyableMessage &) = default;
};

TEST(QueueTest, Create) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();
  ASSERT_EQ(queue.use_count(), 1);
}

TEST(QueueTest, PushAndPop) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();

  auto message = Message{1, "Hello, World!"};
  queue->Push(std::move(message));
  ASSERT_EQ(message.id, 1);
  ASSERT_EQ(message.text, "");

  auto popped = queue->Pop();
  ASSERT_EQ(popped.id, 1);
  ASSERT_EQ(popped.text, "Hello, World!");
}

TEST(QueueTest, CopyableItem) {
  using TO = std::unique_ptr<CopyableMessage>;

  auto queue = kero::mpsc::impl::Queue<TO>::Builder().Build();

  auto message =
      std::make_unique<CopyableMessage>(CopyableMessage{1, "Hello, World!"});
  queue->Push(std::move(message));
  ASSERT_FALSE(message);

  auto popped = queue->Pop();
  ASSERT_EQ(popped->id, 1);
  ASSERT_EQ(popped->text, "Hello, World!");
}

TEST(TxTest, Create) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);
}

TEST(TxTest, Move) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto tx2 = std::move(tx);
  ASSERT_EQ(queue.use_count(), 2);
}

TEST(TxTest, Clone) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto tx2 = tx.Clone();
  ASSERT_EQ(queue.use_count(), 3);
}

TEST(TxTest, Send) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto message = Message{1, "Hello, World!"};
  tx.Send(std::move(message));
  ASSERT_EQ(message.id, 1);
  ASSERT_EQ(message.text, "");
}

TEST(RxTest, Create) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto rx = kero::mpsc::Rx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);
}

TEST(RxText, Move) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto rx = kero::mpsc::Rx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto rx2 = std::move(rx);
  ASSERT_EQ(queue.use_count(), 2);
}

TEST(RxTest, Receive) {
  auto queue = kero::mpsc::impl::Queue<Message>::Builder().Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto rx = kero::mpsc::Rx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 3);

  auto message = Message{1, "Hello, World!"};
  tx.Send(std::move(message));
  ASSERT_EQ(message.id, 1);
  ASSERT_EQ(message.text, "");

  auto popped = rx.Receive();
  ASSERT_EQ(popped.id, 1);
  ASSERT_EQ(popped.text, "Hello, World!");
}

TEST(MpscTest, Create) {
  auto [tx, rx] = kero::mpsc::Channel<Message>::Builder().Build();
}

TEST(MpscTest, SendAndReceive) {
  auto [tx, rx] = kero::mpsc::Channel<Message>::Builder().Build();

  auto message = Message{1, "Hello, World!"};
  tx.Send(std::move(message));
  ASSERT_EQ(message.id, 1);
  ASSERT_EQ(message.text, "");

  auto popped = rx.Receive();
  ASSERT_EQ(popped.id, 1);
  ASSERT_EQ(popped.text, "Hello, World!");
}