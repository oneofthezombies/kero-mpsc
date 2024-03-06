cc_library(
    name = "kero_mpsc",
    hdrs = ["src/kero_mpsc.h"],
    includes = ["src"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "kero_mpsc_test",
    srcs = ["src/kero_mpsc_test.cc"],
    deps = [
        ":kero_mpsc",
        "@googletest//:gtest_main",
    ],
)

cc_binary(
    name = "kero_mpsc_example",
    srcs = ["src/kero_mpsc_example.cc"],
    deps = [":kero_mpsc"],
)

cc_binary(
    name = "kero_mpsc_example_to",
    srcs = ["src/kero_mpsc_example_to.cc"],
    deps = [":kero_mpsc"],
)
