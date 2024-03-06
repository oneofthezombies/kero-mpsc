cc_library(
    name = "kero_mpsc",
    hdrs = ["kero_mpsc.h"],
)

cc_test(
    name = "kero_mpsc_test",
    srcs = ["kero_mpsc_test.cc"],
    deps = [
        ":kero_mpsc",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "kero_mpsc_example",
    srcs = ["kero_mpsc_example.cc"],
    deps = [":kero_mpsc"],
)

cc_binary(
    name = "kero_mpsc_example_to",
    srcs = ["kero_mpsc_example_to.cc"],
    deps = [":kero_mpsc"],
)
