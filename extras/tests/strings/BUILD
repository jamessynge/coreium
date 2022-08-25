cc_test(
    name = "progmem_string_data_test",
    srcs = ["progmem_string_data_test.cc"],
    deps = [
        "//absl/log",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:pretty_type_name",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:test_strings",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
    ],
)

cc_test(
    name = "progmem_string_test",
    srcs = ["progmem_string_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:test_strings",
        "//mcucore/src/strings:progmem_string",
        "//mcucore/src/strings:progmem_string_data",
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
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_compare",
        "//mcucore/src/strings:string_view",
    ],
)

cc_test(
    name = "string_compare_test",
    srcs = ["string_compare_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_compare",
        "//mcucore/src/strings:string_view",
    ],
)

cc_test(
    name = "string_view_test",
    srcs = ["string_view_test.cc"],
    deps = [
        "//absl/log",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:string_view_utils",
        "//mcucore/src/strings:string_view",
    ],
)

cc_test(
    name = "tiny_string_test",
    srcs = ["tiny_string_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:test_strings",
        "//mcucore/src/strings:tiny_string",
    ],
)
