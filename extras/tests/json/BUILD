# Tests of mcucore/src/json/...

cc_test(
    name = "json_encoder_test",
    srcs = ["json_encoder_test.cc"],
    deps = [
        "//absl/log",
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/host/arduino:print",
        "//mcucore/extras/test_tools:json_test_utils",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src/json:json_encoder",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_view",
    ],
)
