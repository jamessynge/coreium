load(
    "//mcucore/extras/dev_tools:build_config.bzl",
    "build_config",
)

package(
    default_copts = build_config.default_copts,
)

cc_library(
    name = "McuCore",
    hdrs = ["McuCore.h"],
    deps = [
        ":any_printable",
        ":array",
        ":array_view",
        ":counting_print",
        ":crc32",
        ":eeprom_io",
        ":has_print_to",
        ":has_progmem_char_array",
        ":hex_escape",
        ":json_encoder",
        ":json_encoder_helpers",
        ":log_sink",
        ":logging",
        ":mcucore_config",
        ":mcucore_platform",
        ":o_print_stream",
        ":print_misc",
        ":printable_cat",
        ":progmem_string",
        ":progmem_string_data",
        ":progmem_string_view",
        ":status",
        ":status_or",
        ":stream_to_print",
        ":string_compare",
        ":string_view",
        ":tiny_string",
        ":type_traits",
    ],
)

cc_library(
    name = "any_printable",
    srcs = ["any_printable.cc"],
    hdrs = ["any_printable.h"],
    deps = [
        ":mcucore_platform",
        ":progmem_string",
        ":progmem_string_view",
        ":string_view",
        ":type_traits",
    ],
)

cc_library(
    name = "array",
    hdrs = ["array.h"],
    deps = [
        ":logging",
        ":mcucore_platform",
    ],
)

cc_library(
    name = "array_view",
    hdrs = ["array_view.h"],
    deps = [
        ":logging",
        ":mcucore_platform",
    ],
)

cc_library(
    name = "counting_print",
    srcs = ["counting_print.cc"],
    hdrs = ["counting_print.h"],
    deps = [":mcucore_platform"],
)

cc_library(
    name = "crc32",
    srcs = ["crc32.cc"],
    hdrs = ["crc32.h"],
    deps = [
        ":logging",
        ":mcucore_platform",
    ],
)

cc_library(
    name = "eeprom_io",
    srcs = ["eeprom_io.cc"],
    hdrs = ["eeprom_io.h"],
    deps = [
        ":crc32",
        ":logging",
        ":mcucore_platform",
        ":progmem_string_view",
    ],
)

cc_library(
    name = "has_print_to",
    hdrs = ["has_print_to.h"],
    deps = [
        ":mcucore_platform",
        ":type_traits",
    ],
)

cc_library(
    name = "has_progmem_char_array",
    hdrs = ["has_progmem_char_array.h"],
    deps = [":type_traits"],
)

cc_library(
    name = "hex_escape",
    srcs = ["hex_escape.cc"],
    hdrs = ["hex_escape.h"],
    deps = [":mcucore_platform"],
)

cc_library(
    name = "json_encoder",
    srcs = ["json_encoder.cc"],
    hdrs = ["json_encoder.h"],
    deps = [
        ":any_printable",
        ":counting_print",
        ":logging",
        ":mcucore_platform",
        ":o_print_stream",
        ":progmem_string_view",
    ],
)

cc_library(
    name = "json_encoder_helpers",
    srcs = ["json_encoder_helpers.cc"],
    hdrs = ["json_encoder_helpers.h"],
    deps = [
        ":array_view",
        ":counting_print",
        ":json_encoder",
        ":logging",
        ":mcucore_platform",
    ],
)

cc_library(
    name = "log_sink",
    srcs = ["log_sink.cc"],
    hdrs = ["log_sink.h"],
    deps = [
        ":mcucore_platform",
        ":o_print_stream",
        ":progmem_string_data",
        "//base:logging_extensions",
        "//glog:logging",
        "//mcucore/extras/test_tools:print_to_std_string",
    ],
)

cc_library(
    name = "logging",
    hdrs = ["logging.h"],
    deps = [
        ":log_sink",
        ":mcucore_config",
        ":o_print_stream",
        ":progmem_string_data",
    ],
)

cc_library(
    name = "mcucore_config",
    hdrs = ["mcucore_config.h"],
)

cc_library(
    name = "mcucore_platform",
    hdrs = ["mcucore_platform.h"],
    deps = [
        ":mcucore_config",
        "//absl/base:core_headers",
        "//mcucore/extras/host/arduino",
        "//mcucore/extras/host/arduino:pgmspace",
        "//mcucore/extras/host/arduino:print",
        "//mcucore/extras/host/arduino:stream",
        "//mcucore/extras/host/eeprom",
    ],
)

cc_library(
    name = "o_print_stream",
    hdrs = ["o_print_stream.h"],
    deps = [
        ":has_print_to",
        ":int_helpers",
        ":mcucore_platform",
        ":progmem_string_data",
        ":type_traits",
    ],
)

cc_library(
    name = "print_misc",
    srcs = ["print_misc.cc"],
    hdrs = ["print_misc.h"],
    deps = [
        ":counting_print",
        ":mcucore_platform",
        ":progmem_string_data",
    ],
)

cc_library(
    name = "printable_cat",
    srcs = ["printable_cat.cc"],
    hdrs = ["printable_cat.h"],
    deps = [
        ":any_printable",
        ":array",
        ":array_view",
        ":mcucore_platform",
    ],
)

cc_library(
    name = "progmem_string",
    srcs = ["progmem_string.cc"],
    hdrs = ["progmem_string.h"],
    deps = [
        ":has_progmem_char_array",
        ":mcucore_platform",
        ":type_traits",
    ],
)

cc_library(
    name = "progmem_string_data",
    hdrs = ["progmem_string_data.h"],
    deps = [":mcucore_platform"],
)

cc_library(
    name = "progmem_string_view",
    srcs = ["progmem_string_view.cc"],
    hdrs = ["progmem_string_view.h"],
    deps = [
        ":has_print_to",
        ":has_progmem_char_array",
        ":logging",
        ":mcucore_platform",
        ":progmem_string_data",
        ":type_traits",
    ],
)

cc_library(
    name = "status",
    srcs = ["status.cc"],
    hdrs = ["status.h"],
    deps = [
        ":counting_print",
        ":hex_escape",
        ":mcucore_platform",
        ":o_print_stream",
        ":progmem_string_data",
        ":progmem_string_view",
    ],
)

cc_library(
    name = "status_or",
    hdrs = ["status_or.h"],
    deps = [
        ":logging",
        ":status",
    ],
)

cc_library(
    name = "stream_to_print",
    hdrs = ["stream_to_print.h"],
    deps = [
        ":mcucore_platform",
        ":o_print_stream",
    ],
)

cc_library(
    name = "string_compare",
    srcs = ["string_compare.cc"],
    hdrs = ["string_compare.h"],
    deps = [
        ":progmem_string_view",
        ":string_view",
    ],
)

cc_library(
    name = "string_view",
    srcs = ["string_view.cc"],
    hdrs = ["string_view.h"],
    deps = [
        ":has_print_to",
        ":has_progmem_char_array",
        ":hex_escape",
        ":logging",
        ":mcucore_platform",
        ":o_print_stream",
    ],
)

cc_library(
    name = "tiny_string",
    hdrs = ["tiny_string.h"],
    deps = [
        ":logging",
        ":mcucore_platform",
    ],
)

cc_library(
    name = "type_traits",
    hdrs = ["type_traits.h"],
    deps = [":mcucore_platform"],
)

cc_library(
    name = "int_helpers",
    hdrs = ["int_helpers.h"],
    deps = [
        ":mcucore_platform",
        ":type_traits",
    ],
)
