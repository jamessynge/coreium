# Tests of mcucore/src/status/...

cc_test(
    name = "status_or_test",
    srcs = ["status_or_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:status_or_test_utils",
        "//mcucore/extras/test_tools:status_test_utils",
        "//mcucore/extras/test_tools/log:check_sink_test_base",
        "//mcucore/src/status",
        "//mcucore/src/status:status_code",
        "//mcucore/src/status:status_or",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_view",
    ],
)

cc_test(
    name = "status_test",
    srcs = ["status_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:progmem_string_view_utils",
        "//mcucore/extras/test_tools:status_test_utils",
        "//mcucore/src/misc:preproc",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/status",
        "//mcucore/src/status:status_code",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
    ],
)
