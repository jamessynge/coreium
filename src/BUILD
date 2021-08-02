cc_library(
    name = "basename",
    hdrs = ["basename.h"],
    deps = [
        ":inline_literal",
        ":mcucore_platform",
    ],
)

cc_library(
    name = "inline_literal",
    hdrs = ["inline_literal.h"],
    deps = [
        ":mcucore_platform",
        ":progmem_string_view",
    ],
)

cc_library(
    name = "log_sink",
    srcs = ["log_sink.cc"],
    hdrs = ["log_sink.h"],
    deps = [
        ":mcucore_platform",
        ":o_print_stream",
        "//base:logging_extensions",
        "//extras/test_tools:print_to_std_string",
        "//glog:logging",
    ],
)

cc_library(
    name = "logging",
    hdrs = ["logging.h"],
    deps = [
        ":basename",
        ":log_sink",
        ":mcucore_config",
    ],
)

cc_library(
    name = "mcucore_config",
    hdrs = ["mcucore_config.h"],
)

cc_library(
    name = "McuCore",
    hdrs = ["McuCore.h"],
    deps = [
        ":basename",
        ":inline_literal",
        ":log_sink",
        ":logging",
        ":mcucore_config",
        ":mcucore_platform",
        ":o_print_stream",
        ":print_to_trait",
        ":progmem_string",
        ":progmem_string_view",
        ":type_traits",
    ],
)

cc_library(
    name = "mcucore_platform",
    hdrs = ["mcucore_platform.h"],
    deps = [
        ":mcucore_config",
        "//absl/base:core_headers",
        "//extras/host/arduino",
        "//extras/host/arduino:pgmspace",
        "//extras/host/arduino:print",
        "//extras/host/arduino:stream",
        "//extras/host/eeprom",
    ],
)

cc_library(
    name = "o_print_stream",
    hdrs = ["o_print_stream.h"],
    deps = [
        ":mcucore_platform",
        ":print_to_trait",
        ":type_traits",
    ],
)

cc_library(
    name = "print_to_trait",
    hdrs = ["print_to_trait.h"],
    deps = [
        ":mcucore_platform",
        ":type_traits",
    ],
)

cc_library(
    name = "progmem_string",
    srcs = ["progmem_string.cc"],
    hdrs = ["progmem_string.h"],
    deps = [
        ":inline_literal",
        ":mcucore_platform",
    ],
)

cc_library(
    name = "progmem_string_view",
    srcs = ["progmem_string_view.cc"],
    hdrs = ["progmem_string_view.h"],
    deps = [
        ":mcucore_platform",
        ":print_to_trait",
    ],
)

cc_library(
    name = "type_traits",
    hdrs = ["type_traits.h"],
    deps = [":mcucore_platform"],
)
