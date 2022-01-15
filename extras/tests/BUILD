# Contains tests of code that is in the src directory, to be executed on host.
# We don't put the tests in the src directory because otherwise the Arduino IDE
# treats them as code to be compiled for the target microcontroller, which fails.

cc_test(
    name = "any_printable_test",
    srcs = ["any_printable_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/host/arduino:print",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src:any_printable",
        "//mcucore/src:progmem_string_data",
        "//mcucore/src:progmem_string_view",
        "//mcucore/src:string_view",
    ],
)

cc_test(
    name = "array_test",
    srcs = ["array_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src:array",
    ],
)

cc_test(
    name = "array_view_test",
    srcs = ["array_view_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src:array_view",
    ],
)

cc_test(
    name = "counting_print_test",
    srcs = ["counting_print_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src:counting_print",
    ],
)

cc_test(
    name = "crc32_test",
    srcs = ["crc32_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:random_utils",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src:crc32",
    ],
)

cc_test(
    name = "eeprom_io_test",
    srcs = ["eeprom_io_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/host/eeprom",
        "//mcucore/extras/test_tools:eeprom_test_utils",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/src:crc32",
        "//mcucore/src:eeprom_io",
        "//mcucore/src:progmem_string_view",
    ],
)

cc_test(
    name = "hex_escape_test",
    srcs = ["hex_escape_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src:hex_escape",
        "//mcucore/src:string_view",
        "//util/gtl:map_util",
    ],
)

cc_test(
    name = "json_encoder_test",
    srcs = ["json_encoder_test.cc"],
    deps = [
        "//absl/strings",
        "//glog:logging",
        "//googletest:gunit_main",
        "//mcucore/extras/host/arduino:print",
        "//mcucore/extras/test_tools:json_test_utils",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src:json_encoder",
        "//mcucore/src:progmem_string_view",
        "//mcucore/src:string_view",
    ],
)

cc_test(
    name = "log_sink_test",
    srcs = ["log_sink_test.cc"],
    deps = [
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src:log_sink",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src:o_print_stream",
        "//mcucore/src:progmem_string_data",
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
    ],
)

cc_test(
    name = "o_print_stream_test",
    srcs = ["o_print_stream_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src:o_print_stream",
    ],
)

cc_test(
    name = "printable_cat_test",
    srcs = ["printable_cat_test.cc"],
    deps = [
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/src:any_printable",
        "//mcucore/src:printable_cat",
        "//mcucore/src:progmem_string",
        "//mcucore/src:progmem_string_view",
        "//mcucore/src:string_view",
    ],
)

cc_test(
    name = "progmem_string_data_test",
    srcs = ["progmem_string_data_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:test_strings",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src:o_print_stream",
        "//mcucore/src:progmem_string_data",
        "//mcucore/src:progmem_string_view",
    ],
)

cc_test(
    name = "progmem_string_test",
    srcs = ["progmem_string_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:test_strings",
        "//mcucore/src:progmem_string",
        "//mcucore/src:progmem_string_data",
    ],
)

cc_test(
    name = "progmem_string_view_test",
    srcs = ["progmem_string_view_test.cc"],
    deps = [
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:progmem_string_view_utils",
        "//mcucore/extras/test_tools:string_view_utils",
        "//mcucore/extras/test_tools:test_strings",
        "//mcucore/src:hex_escape",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src:o_print_stream",
        "//mcucore/src:progmem_string_view",
        "//mcucore/src:string_compare",
        "//mcucore/src:string_view",
    ],
)

cc_test(
    name = "status_or_test",
    srcs = ["status_or_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:status_or_test_utils",
        "//mcucore/src:status",
        "//mcucore/src:status_or",
        "//mcucore/src:string_view",
    ],
)

cc_test(
    name = "status_test",
    srcs = ["status_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:progmem_string_view_utils",
        "//mcucore/src:status",
    ],
)

cc_test(
    name = "stream_to_print_test",
    srcs = ["stream_to_print_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/host/arduino:print",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src:o_print_stream",
        "//mcucore/src:stream_to_print",
    ],
)

cc_test(
    name = "string_view_test",
    srcs = ["string_view_test.cc"],
    deps = [
        "//glog:logging",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:string_view_utils",
        "//mcucore/src:string_view",
    ],
)

cc_test(
    name = "type_traits_test",
    srcs = ["type_traits_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src:type_traits",
    ],
)

cc_test(
    name = "string_compare_test",
    srcs = ["string_compare_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src:string_compare",
    ],
)
