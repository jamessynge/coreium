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
        "//extras/host/arduino:ip_address",
        "//extras/host/ethernet5500:ethernet_client",
        "//googletest:gunit_headers",
    ],
)

cc_library(
    name = "print_to_std_string",
    hdrs = ["print_to_std_string.h"],
    deps = [
        "//extras/host/arduino:print",
    ],
)

cc_library(
    name = "sample_printable",
    hdrs = ["sample_printable.h"],
    deps = ["//extras/host/arduino:print"],
)
