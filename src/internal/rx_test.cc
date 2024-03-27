#include "internal/rx.h"
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

TEST(RxTest, Create) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto rx = kero::mpsc::Rx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);
}

TEST(RxText, Move) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto rx = kero::mpsc::Rx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto rx2 = std::move(rx);
  ASSERT_EQ(queue.use_count(), 2);
}

TEST(RxTest, Receive) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
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

TEST(RxTest, TryReceive) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto rx = kero::mpsc::Rx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 3);

  {
    auto popped = rx.TryReceive();
    ASSERT_FALSE(popped);
  }

  {
    auto message = Message{1, "Hello, World!"};
    tx.Send(std::move(message));
    ASSERT_EQ(message.id, 1);
    ASSERT_EQ(message.text, "");
  }

  {
    auto popped = rx.TryReceive();
    ASSERT_TRUE(popped);
    ASSERT_EQ(popped->id, 1);
    ASSERT_EQ(popped->text, "Hello, World!");
  }

  {
    auto popped = rx.TryReceive();
    ASSERT_FALSE(popped);
  }
}

TEST(RxTest, TryReceiveAll) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);

  auto tx = kero::mpsc::Tx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 2);

  auto rx = kero::mpsc::Rx<Message>{queue};
  ASSERT_EQ(queue.use_count(), 3);

  {
    auto poppeds = rx.TryReceiveAll();
    ASSERT_EQ(poppeds.size(), 0);
  }

  {
    auto message = Message{1, "Hello, World!"};
    tx.Send(std::move(message));
    ASSERT_EQ(message.id, 1);
    ASSERT_EQ(message.text, "");
  }

  {
    auto poppeds = rx.TryReceiveAll();
    ASSERT_EQ(poppeds.size(), 1);
    auto popped = std::move(poppeds.front());
    ASSERT_EQ(popped.id, 1);
    ASSERT_EQ(popped.text, "Hello, World!");
  }

  {
    auto poppeds = rx.TryReceiveAll();
    ASSERT_EQ(poppeds.size(), 0);
  }

  {
    auto message = Message{1, "Hello, World!"};
    tx.Send(std::move(message));
    ASSERT_EQ(message.id, 1);
    ASSERT_EQ(message.text, "");
  }

  {
    auto message = Message{2, "Hello, World!"};
    tx.Send(std::move(message));
    ASSERT_EQ(message.id, 2);
    ASSERT_EQ(message.text, "");
  }

  {
    auto poppeds = rx.TryReceiveAll();
    ASSERT_EQ(poppeds.size(), 2);
    auto popped = std::move(poppeds.front());
    ASSERT_EQ(popped.id, 1);
    ASSERT_EQ(popped.text, "Hello, World!");
    poppeds.pop();
    popped = std::move(poppeds.front());
    ASSERT_EQ(popped.id, 2);
    ASSERT_EQ(popped.text, "Hello, World!");
  }

  {
    auto poppeds = rx.TryReceiveAll();
    ASSERT_EQ(poppeds.size(), 0);
  }
}
