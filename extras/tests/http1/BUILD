# Tests of mcucore/src/http1/...

cc_test(
    name = "request_decoder_internals_test",
    srcs = ["request_decoder_internals_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src/http1:request_decoder",
    ],
)

cc_test(
    name = "request_decoder_test",
    srcs = ["request_decoder_test.cc"],
    deps = [
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools/http1:collapsing_request_decoder_listener",
        "//mcucore/extras/test_tools/http1:mock_request_decoder_listener",
        "//mcucore/extras/test_tools/http1:string_utils",
        "//mcucore/src/http1:request_decoder",
        "//mcucore/src/http1:request_decoder_constants",
        "//mcucore/src/strings:string_view",
    ],
)

cc_test(
    name = "request_router_test",
    srcs = ["request_router_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
        "//mcucore/src/http1:request_decoder",
        "//mcucore/src/http1:request_router",
        "//mcucore/src/strings:progmem_string_data",
    ],
)
