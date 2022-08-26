# Tests of mcucore/src/log/...

cc_test(
    name = "log_sink_test",
    srcs = ["log_sink_test.cc"],
    deps = [
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log:log_sink",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

cc_test(
    name = "log_test",
    srcs = ["log_test.cc"],
    deps = [
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src/log",
        "//mcucore/src/strings:progmem_string_view",
    ],
)
