# Kero MPSC

A memory-safe "multiple producer single consumer queue" implementation written in C++.  

## Why I Created This

Memory safety in programming languages is no longer an option. This is an area that must be met.  
However, the status of the C++ programming language in the current compiler industry is absolute. So I wrote the code in C++, but developed it to meet memory safety features.  

## How to Use

Create `tx` and `rx` using `kero::mpsc::mpsc<T>()` function.  
When you call `tx->send(item)`, an object is sent.  
You can receive an object by calling `rx->receive()`. At this time, blocking occurs until a object is received.

```cpp
// create message passing channel
auto [tx, rx] = kero::mpsc::mpsc<MyMessage>();

// create thread to send message
std::thread sender([tx = std::move(tx)] {
  auto message = MyMessage{1, "Hello, World!"};
  tx->send(std::move(message));
});

// receive message
auto message = rx->receive();
assert(message.id == 1);
assert(message.text == "Hello, World!");
```

Please see the full example on [kero_mpsc_example.cc](kero_mpsc/kero_mpsc_example.cc).  

If the object type you want to transfer is not move only, you can use `std::unique_ptr` as the transfer object.  
For an example of using `std::unique_ptr` as a transfer object, please refer to [kero_mpsc_example_to.cc](kero_mpsc/kero_mpsc_example_to.cc).

## For Contributors

This project was developed based on Bazel.  
Please ensure `bazel` command and after calling the `bazel --version` command, check whether the version is higher than `7.0.0`.  

You can get IDE support by running the command below.  

```sh
bazel run @hedron_compile_commands//:refresh_all
```

For more information, please refer to the link below.  

https://github.com/hedronvision/bazel-compile-commands-extractor

## References

[February 2024 Whitehouse Technical Report](https://www.whitehouse.gov/wp-content/uploads/2024/02/Final-ONCD-Technical-Report.pdf)  
