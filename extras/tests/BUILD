# Contains tests of code that is in the src directory, to be executed on host.
# We don't put the tests in the src directory because otherwise the Arduino IDE
# treats them as code to be compiled for the target microcontroller, which fails.

cc_library(
    name = "basename_test",
    srcs = ["basename_test.cc"],
    deps = [
        "//googletest:gunit_headers",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src:basename",
        "//mcucore/src:inline_literal",
    ],
)

cc_library(
    name = "inline_literal_test",
    srcs = ["inline_literal_test.cc"],
    deps = [
        "//googletest:gunit_headers",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src:inline_literal",
        "//mcucore/src:o_print_stream",
        "//mcucore/src:progmem_string_view",
        "//tas/src/utils:literal",
    ],
)

cc_library(
    name = "log_sink_test",
    srcs = ["log_sink_test.cc"],
    deps = [
        "//googletest:gunit_headers",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src:inline_literal",
        "//mcucore/src:log_sink",
        "//mcucore/src:o_print_stream",
    ],
)

cc_test(
    name = "mcucore_config_test",
    srcs = ["mcucore_config_test.cc"],
    deps = [
        "//glog:logging",
        "//googletest:gunit_main",
        "//mcucore/src:mcucore_config",
        "//mcucore/src:mcucore_platform",
        "//tas/src:config",
    ],
)

cc_library(
    name = "o_print_stream_test",
    srcs = ["o_print_stream_test.cc"],
    deps = [
        "//googletest:gunit_headers",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src:o_print_stream",
    ],
)

cc_library(
    name = "progmem_string_view_test",
    srcs = ["progmem_string_view_test.cc"],
    deps = [
        "//absl/strings",
        "//googletest:gunit_headers",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src:o_print_stream",
        "//mcucore/src:progmem_string_view",
        "//tas/extras/test_tools:progmem_string_view_utils",
        "//tas/extras/test_tools:string_view_utils",
        "//tas/src/utils:hex_escape",
        "//tas/src/utils:string_compare",
        "//tas/src/utils:string_view",
    ],
)
