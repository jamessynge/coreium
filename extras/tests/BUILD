# Contains tests of code that is in the src directory, to be executed on host.
# We don't put the tests in the src directory because otherwise the Arduino IDE
# treats them as code to be compiled for the target microcontroller, which fails.

cc_library(
    name = "basename_test",
    srcs = ["basename_test.cc"],
    deps = [
        "//extras/test_tools:print_to_std_string",
        "//googletest:gunit_headers",
        "//src:basename",
        "//src:inline_literal",
    ],
)

cc_library(
    name = "inline_literal_test",
    srcs = ["inline_literal_test.cc"],
    deps = [
        "//experimental/users/jamessynge/arduino/tas/src/utils:literal",
        "//extras/test_tools:print_to_std_string",
        "//googletest:gunit_headers",
        "//src:inline_literal",
        "//src:o_print_stream",
        "//src:progmem_string_view",
    ],
)

cc_library(
    name = "log_sink_test",
    srcs = ["log_sink_test.cc"],
    deps = [
        "//extras/test_tools:print_to_std_string",
        "//googletest:gunit_headers",
        "//src:inline_literal",
        "//src:log_sink",
        "//src:o_print_stream",
    ],
)

cc_test(
    name = "mcucore_config_test",
    srcs = ["mcucore_config_test.cc"],
    deps = [
        "//experimental/users/jamessynge/arduino/tas/src:config",
        "//glog:logging",
        "//googletest:gunit_main",
        "//src:mcucore_config",
        "//src:mcucore_platform",
    ],
)

cc_library(
    name = "o_print_stream_test",
    srcs = ["o_print_stream_test.cc"],
    deps = [
        "//extras/test_tools:print_to_std_string",
        "//extras/test_tools:sample_printable",
        "//googletest:gunit_headers",
        "//src:o_print_stream",
    ],
)

cc_library(
    name = "progmem_string_view_test",
    srcs = ["progmem_string_view_test.cc"],
    deps = [
        "//absl/strings",
        "//experimental/users/jamessynge/arduino/tas/extras/test_tools:progmem_string_view_utils",
        "//experimental/users/jamessynge/arduino/tas/extras/test_tools:string_view_utils",
        "//experimental/users/jamessynge/arduino/tas/src/utils:hex_escape",
        "//experimental/users/jamessynge/arduino/tas/src/utils:string_compare",
        "//experimental/users/jamessynge/arduino/tas/src/utils:string_view",
        "//extras/test_tools:print_to_std_string",
        "//googletest:gunit_headers",
        "//src:mcucore_platform",
        "//src:o_print_stream",
        "//src:progmem_string_view",
    ],
)
