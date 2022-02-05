# Utilities intended to simplify the task of writing unit tests of McuCore.

package(
    # In addition to testonly, I'd also like to have a "hostonly" feature on a
    # target, so that I could readily mark those features that can be used on
    # embedded systems, and those that are only available on host systems. There
    # probably is some notion of "platform" that I can use. TBD.
    default_testonly = 1,
)

cc_library(
    name = "case_insensitive_less",
    srcs = ["case_insensitive_less.cc"],
    hdrs = ["case_insensitive_less.h"],
)

cc_library(
    name = "eeprom_test_utils",
    srcs = ["eeprom_test_utils.cc"],
    hdrs = ["eeprom_test_utils.h"],
    deps = [
        "//absl/strings",
        "//absl/strings:str_format",
        "//googletest:gunit_headers",
        "//mcucore/extras/host/eeprom",
    ],
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
    name = "mock_ethernet_client",
    hdrs = ["mock_ethernet_client.h"],
    deps = [
        "//googletest:gunit_headers",
        "//mcucore/extras/host/arduino:ip_address",
        "//mcunet/extras/host/ethernet5500:ethernet_client",
    ],
)

cc_library(
    name = "pretty_type_name",
    srcs = ["pretty_type_name.cc"],
    hdrs = ["pretty_type_name.h"],
)

cc_test(
    name = "pretty_type_name_test",
    srcs = ["pretty_type_name_test.cc"],
    deps = [
        ":pretty_type_name",
        "//googletest:gunit_main",
    ],
)

cc_library(
    name = "print_to_std_string",
    testonly = 0,
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
    deps = [
        ":print_value_to_std_string",
        "//mcucore/src:progmem_string_view",
    ],
)

cc_library(
    name = "random_utils",
    srcs = ["random_utils.cc"],
    hdrs = ["random_utils.h"],
    deps = [
        "//absl/flags:flag",
        "//glog:logging",
    ],
)

cc_library(
    name = "sample_printable",
    hdrs = ["sample_printable.h"],
    deps = ["//mcucore/extras/host/arduino:print"],
)

cc_library(
    name = "status_or_test_utils",
    hdrs = ["status_or_test_utils.h"],
    deps = [
        ":print_value_to_std_string",
        ":status_test_utils",
        "//mcucore/src:status_or",
    ],
)

cc_library(
    name = "status_test_utils",
    hdrs = ["status_test_utils.h"],
    deps = [
        ":print_value_to_std_string",
        "//mcucore/src:status",
    ],
)

cc_library(
    name = "string_view_utils",
    hdrs = ["string_view_utils.h"],
    deps = ["//mcucore/src:string_view"],
)

cc_library(
    name = "test_strings",
    srcs = ["test_strings.cc"],
    hdrs = ["test_strings.h"],
)
