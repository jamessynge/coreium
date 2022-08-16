# TODO(jamessynge): describe this package.

cc_library(
    name = "log",
    hdrs = ["log.h"],
    deps = [
        ":log_sink",
        "//mcucore/src:mcucore_config",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

cc_library(
    name = "log_sink",
    srcs = ["log_sink.cc"],
    hdrs = ["log_sink.h"],
    deps = [
        "//base:logging_extensions",
        "//glog:logging",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/strings:progmem_string_data",
    ],
)
