# Experiments designed to help me understand ... stuff.

cc_library(
    name = "jitter_analyzer",
    srcs = ["jitter_analyzer.cc"],
    hdrs = ["jitter_analyzer.h"],
    deps = [
        "//absl/log",
        "//absl/log:check",
    ],
)

cc_test(
    name = "jitter_analyzer_test",
    srcs = ["jitter_analyzer_test.cc"],
    deps = [
        ":jitter_analyzer",
        "//absl/container:flat_hash_map",
        "//googletest:gunit_main",
    ],
)
