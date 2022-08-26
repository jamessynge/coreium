# Provides hashing of byte sequences.

cc_library(
    name = "crc32",
    srcs = ["crc32.cc"],
    hdrs = ["crc32.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
    ],
)

cc_library(
    name = "fnv1a",
    srcs = ["fnv1a.cc"],
    hdrs = ["fnv1a.h"],
    deps = ["//mcucore/src/log"],
)
