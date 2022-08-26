# Tests of mcucore/src/hash/...

cc_test(
    name = "crc32_test",
    srcs = ["crc32_test.cc"],
    deps = [
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:random_utils",
        "//mcucore/src/hash:crc32",
    ],
)

cc_test(
    name = "fnv1a_test",
    srcs = ["fnv1a_test.cc"],
    deps = [
        "//absl/container:btree",
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:random_utils",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src/hash:fnv1a",
        "//mcucore/src/log",
    ],
)
