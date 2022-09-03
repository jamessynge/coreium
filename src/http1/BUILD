# Package provides support for decoding HTTP/1.1 requests.

cc_library(
    name = "request_decoder",
    srcs = ["request_decoder.cc"],
    hdrs = ["request_decoder.h"],
    deps = [
        ":request_decoder_constants",
        "//mcucore/src/log",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/print:print_misc",
        "//mcucore/src/strings:progmem_string",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_compare",
        "//mcucore/src/strings:string_view",
    ],
)

cc_library(
    name = "request_decoder_constants",
    srcs = ["request_decoder_constants.cc"],
    hdrs = ["request_decoder_constants.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/container:flash_string_table",
        "//mcucore/src/print:print_misc",
        "//mcucore/src/print:print_to_buffer",
        "//mcucore/src/strings:progmem_string_data",
    ],
)
