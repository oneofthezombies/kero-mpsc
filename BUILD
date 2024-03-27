cc_library(
    name = "kero_mpsc",
    srcs = [
        "src/internal/channel.h",
        "src/internal/core.h",
        "src/internal/queue.h",
        "src/internal/rx.h",
        "src/internal/tx.h",
    ],
    hdrs = ["src/kero_mpsc.h"],
    copts = ["-std=c++20"],
    includes = ["src"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "kero_mpsc_test",
    srcs = [
        "src/internal/channel_test.cc",
        "src/internal/queue_test.cc",
        "src/internal/rx_test.cc",
        "src/internal/tx_test.cc",
    ],
    copts = ["-std=c++20"],
    deps = [
        ":kero_mpsc",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "kero_mpsc_example",
    srcs = ["src/kero_mpsc_example.cc"],
    copts = ["-std=c++20"],
    deps = [":kero_mpsc"],
)

cc_binary(
    name = "kero_mpsc_example_to",
    srcs = ["src/kero_mpsc_example_to.cc"],
    copts = ["-std=c++20"],
    deps = [":kero_mpsc"],
)
