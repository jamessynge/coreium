# Utilities intended to simplify the task of writing unit tests of RequestDecoder
# and related features of McuCore.

package(
    # In addition to testonly, I'd also like to have a "hostonly" feature on a
    # target, so that I could readily mark those features that can be used on
    # embedded systems, and those that are only available on host systems. There
    # probably is some notion of "platform" that I can use. TBD.
    default_testonly = 1,
)

cc_library(
    name = "collapsing_request_decoder_listener",
    srcs = ["collapsing_request_decoder_listener.cc"],
    hdrs = ["collapsing_request_decoder_listener.h"],
    deps = [
        ":string_utils",
        "//googletest:gunit_headers",
        "//mcucore/extras/test_tools:string_view_utils",
        "//mcucore/src/http1:request_decoder",
        "//mcucore/src/http1:request_decoder_constants",
        "//mcucore/src/strings:string_view",
    ],
)

cc_library(
    name = "decode_chunk_encoded",
    srcs = ["decode_chunk_encoded.cc"],
    hdrs = ["decode_chunk_encoded.h"],
    deps = [
        "//absl/status",
        "//absl/status:statusor",
        "//absl/strings",
        "//util/task:status",
    ],
)

cc_test(
    name = "decode_chunk_encoded_test",
    srcs = ["decode_chunk_encoded_test.cc"],
    deps = [
        ":decode_chunk_encoded",
        "//absl/status",
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:print_to_std_string",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:sample_printable",
    ],
)

cc_library(
    name = "mock_request_decoder_listener",
    srcs = ["mock_request_decoder_listener.cc"],
    hdrs = ["mock_request_decoder_listener.h"],
    deps = [
        "//googletest:gunit_headers",
        "//mcucore/extras/test_tools:progmem_string_utils",
        "//mcucore/extras/test_tools:string_view_utils",
        "//mcucore/src/http1:request_decoder",
        "//mcucore/src/http1:request_decoder_constants",
        "//mcucore/src/strings:string_view",
    ],
)

cc_library(
    name = "string_utils",
    srcs = ["string_utils.cc"],
    hdrs = ["string_utils.h"],
    deps = [
        "//absl/log",
        "//absl/log:check",
        "//absl/strings",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/strings:string_view",
    ],
)
