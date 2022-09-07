# Tests of container classes.

cc_test(
    name = "array_test",
    srcs = ["array_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src/container:array",
    ],
)

cc_test(
    name = "array_view_test",
    srcs = ["array_view_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src/container:array_view",
    ],
)

cc_test(
    name = "flash_string_table_test",
    srcs = ["flash_string_table_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src/container:flash_string_table",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

cc_test(
    name = "serial_map_test",
    srcs = ["serial_map_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:status_or_test_utils",
        "//mcucore/extras/test_tools:status_test_utils",
        "//mcucore/extras/test_tools:string_view_utils",
        "//mcucore/extras/test_tools:test_has_failed",
        "//mcucore/src/container:serial_map",
        "//mcucore/src/strings:progmem_string",
        "//mcucore/src/strings:progmem_string_data",
    ],
)
