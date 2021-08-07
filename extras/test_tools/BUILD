# Utilities intended to simplify the task of writing unit tests of Arduino libraries.

cc_library(
    name = "case_insensitive_less",
    srcs = ["case_insensitive_less.cc"],
    hdrs = ["case_insensitive_less.h"],
)

cc_library(
    name = "mock_ethernet_client",
    hdrs = ["mock_ethernet_client.h"],
    deps = [
        "//googletest:gunit_headers",
        "//mcucore/extras/host/arduino:ip_address",
        "//mcucore/extras/host/ethernet5500:ethernet_client",
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
    name = "sample_printable",
    hdrs = ["sample_printable.h"],
    deps = ["//mcucore/extras/host/arduino:print"],
)
