# Support for generating JSON in a streaming fashion, where there is no buffering of
# the output JSON, making it suitable for a low memory environment.

cc_library(
    name = "json_encoder",
    srcs = ["json_encoder.cc"],
    hdrs = ["json_encoder.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/print:any_printable",
        "//mcucore/src/print:counting_print",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/strings:progmem_string_view",
    ],
)

cc_library(
    name = "json_encoder_helpers",
    srcs = ["json_encoder_helpers.cc"],
    hdrs = ["json_encoder_helpers.h"],
    deps = [
        ":json_encoder",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/container:array_view",
        "//mcucore/src/log",
        "//mcucore/src/print:counting_print",
    ],
)
