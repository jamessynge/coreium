# Provides MCU_VLOG, MCU_CHECK, MCU_DCHECK, etc.

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
        "//absl/log",
        "//absl/log:log_sink_registry",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/strings:progmem_string",
        "//mcucore/src/strings:progmem_string_data",
    ],
)
