# Utilities intended to simplify the task of writing unit tests of Arduino libraries.

cc_library(
    name = "case_insensitive_less",
    srcs = ["case_insensitive_less.cc"],
    hdrs = ["case_insensitive_less.h"],
)

cc_library(
    name = "http_request",
    srcs = ["http_request.cc"],
    hdrs = ["http_request.h"],
    deps = [
        ":case_insensitive_less",
        "//absl/status",
        "//absl/status:statusor",
        "//absl/strings",
    ],
)

cc_library(
    name = "http_response",
    srcs = ["http_response.cc"],
    hdrs = ["http_response.h"],
    deps = [
        ":case_insensitive_less",
        ":json_decoder",
        "//absl/status",
        "//absl/status:statusor",
        "//absl/strings",
        "//util/task:status",
    ],
)

cc_test(
    name = "http_response_test",
    srcs = ["http_response_test.cc"],
    deps = [
        ":http_response",
        ":json_decoder",
        "//absl/status",
        "//absl/status:statusor",
        "//googletest:gunit_main",
        "//util/task:status",
    ],
)

cc_library(
    name = "json_decoder",
    srcs = ["json_decoder.cc"],
    hdrs = ["json_decoder.h"],
    deps = [
        "//absl/status",
        "//absl/status:statusor",
        "//absl/strings",
        "//glog:logging",
        "//googletest:gunit_headers",
        "//mcucore/src:logging",
        "//util/task:status",
    ],
)

cc_test(
    name = "json_decoder_test",
    srcs = ["json_decoder_test.cc"],
    deps = [
        ":json_decoder",
        "//absl/status:statusor",
        "//googletest:gunit_main",
        "//util/task:status",
    ],
)

cc_library(
    name = "json_test_utils",
    srcs = ["json_test_utils.cc"],
    hdrs = ["json_test_utils.h"],
    deps = [
        "//mcucore/extras/host/arduino:print",
        "//mcucore/src:any_printable",
        "//mcucore/src:json_encoder",
        "//mcucore/src:mcucore_platform",
    ],
)

cc_library(
    name = "literal_utils",
    hdrs = ["literal_utils.h"],
    deps = ["//mcucore/src:literal"],
)

cc_library(
    name = "mock_ethernet_client",
    hdrs = ["mock_ethernet_client.h"],
    deps = [
        "//googletest:gunit_headers",
        "//mcucore/extras/host/arduino:ip_address",
        "//mcunet/extras/host/ethernet5500:ethernet_client",
    ],
)

cc_library(
    name = "print_to_std_string",
    hdrs = ["print_to_std_string.h"],
    deps = [
        "//mcucore/extras/host/arduino:print",
    ],
)

cc_library(
    name = "print_value_to_std_string",
    hdrs = ["print_value_to_std_string.h"],
    deps = [
        ":print_to_std_string",
        "//mcucore/src:o_print_stream",
    ],
)

cc_library(
    name = "progmem_string_view_utils",
    hdrs = ["progmem_string_view_utils.h"],
    deps = ["//mcucore/src:progmem_string_view"],
)

cc_library(
    name = "sample_printable",
    hdrs = ["sample_printable.h"],
    deps = ["//mcucore/extras/host/arduino:print"],
)

cc_library(
    name = "string_view_utils",
    hdrs = ["string_view_utils.h"],
    deps = ["//mcucore/src:string_view"],
)
