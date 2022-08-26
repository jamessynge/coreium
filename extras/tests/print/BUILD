# Tests of mcucore/src/print/...

cc_test(
    name = "any_printable_test",
    srcs = ["any_printable_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/host/arduino:print",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src/print:any_printable",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_view",
    ],
)

cc_test(
    name = "counting_print_test",
    srcs = ["counting_print_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src/print:counting_print",
    ],
)

cc_test(
    name = "has_insert_into_test",
    srcs = ["has_insert_into_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/src/print:has_insert_into",
        "//mcucore/src/print:o_print_stream",
    ],
)

cc_test(
    name = "hex_escape_test",
    srcs = ["hex_escape_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_view",
        "//util/gtl:map_util",
    ],
)

cc_test(
    name = "o_print_stream_test",
    srcs = ["o_print_stream_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/extras/test_tools:test_strings",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/strings:progmem_string",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
    ],
)

cc_test(
    name = "print_to_buffer_test",
    srcs = ["print_to_buffer_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:test_strings",
        "//mcucore/src/print:print_to_buffer",
    ],
)

cc_test(
    name = "printable_cat_test",
    srcs = ["printable_cat_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/src/print:any_printable",
        "//mcucore/src/print:printable_cat",
        "//mcucore/src/strings:progmem_string",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_view",
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
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/print:stream_to_print",
    ],
)
