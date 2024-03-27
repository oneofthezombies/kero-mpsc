#include "internal/queue.h"
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

struct CopyableMessage {
  int id;
  std::string text;

  CopyableMessage(int id, std::string&& text) noexcept
      : id(id), text(std::move(text)) {}

  CopyableMessage(const CopyableMessage&) noexcept = default;
  ~CopyableMessage() noexcept = default;
  auto operator=(const CopyableMessage&) noexcept -> CopyableMessage& = default;
};

TEST(QueueTest, Create) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();
  ASSERT_EQ(queue.use_count(), 1);
}

TEST(QueueTest, PushAndPop) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();

  auto message = Message{1, "Hello, World!"};
  queue->Push(std::move(message));
  ASSERT_EQ(message.id, 1);
  ASSERT_EQ(message.text, "");

  auto popped = queue->Pop();
  ASSERT_EQ(popped.id, 1);
  ASSERT_EQ(popped.text, "Hello, World!");
}

TEST(QueueTest, TryPop) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();

  {
    auto popped = queue->TryPop();
    ASSERT_FALSE(popped);
  }

  {
    auto message = Message{1, "Hello, World!"};
    queue->Push(std::move(message));
    ASSERT_EQ(message.id, 1);
    ASSERT_EQ(message.text, "");
  }

  {
    auto popped = queue->TryPop();
    ASSERT_TRUE(popped);
    ASSERT_EQ(popped->id, 1);
    ASSERT_EQ(popped->text, "Hello, World!");
  }

  {
    auto popped = queue->TryPop();
    ASSERT_FALSE(popped);
  }
}

TEST(QueueTest, TryPopAll) {
  auto queue = kero::mpsc::Queue<Message>::Builder{}.Build();

  {
    auto poppeds = queue->TryPopAll();
    ASSERT_EQ(poppeds.size(), 0);
  }

  {
    auto message = Message{1, "Hello, World!"};
    queue->Push(std::move(message));
    ASSERT_EQ(message.id, 1);
    ASSERT_EQ(message.text, "");
  }

  {
    auto poppeds = queue->TryPopAll();
    ASSERT_EQ(poppeds.size(), 1);
    auto popped = std::move(poppeds.front());
    ASSERT_EQ(popped.id, 1);
    ASSERT_EQ(popped.text, "Hello, World!");
  }

  {
    auto poppeds = queue->TryPopAll();
    ASSERT_EQ(poppeds.size(), 0);
  }

  {
    auto message = Message{1, "Hello, World!"};
    queue->Push(std::move(message));
    ASSERT_EQ(message.id, 1);
    ASSERT_EQ(message.text, "");
  }

  {
    auto message = Message{2, "Hello, World!"};
    queue->Push(std::move(message));
    ASSERT_EQ(message.id, 2);
    ASSERT_EQ(message.text, "");
  }

  {
    auto poppeds = queue->TryPopAll();
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
    auto poppeds = queue->TryPopAll();
    ASSERT_EQ(poppeds.size(), 0);
  }
}

TEST(QueueTest, CopyableItem) {
  using TO = std::unique_ptr<CopyableMessage>;

  auto queue = kero::mpsc::Queue<TO>::Builder{}.Build();

  auto message =
      std::make_unique<CopyableMessage>(CopyableMessage{1, "Hello, World!"});
  queue->Push(std::move(message));
  ASSERT_FALSE(message);

  auto popped = queue->Pop();
  ASSERT_EQ(popped->id, 1);
  ASSERT_EQ(popped->text, "Hello, World!");
}
