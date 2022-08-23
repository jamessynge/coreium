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
        ":mcucore_config",
        ":mcucore_platform",
        "//mcucore/src/container:array",
        "//mcucore/src/container:array_view",
        "//mcucore/src/container:flash_string_table",
        "//mcucore/src/container:serial_map",
        "//mcucore/src/eeprom:eeprom_io",
        "//mcucore/src/eeprom:eeprom_region",
        "//mcucore/src/eeprom:eeprom_tag",
        "//mcucore/src/eeprom:eeprom_tlv",
        "//mcucore/src/hash:crc32",
        "//mcucore/src/hash:fnv1a",
        "//mcucore/src/http1:request_decoder",
        "//mcucore/src/http1:request_decoder_constants",
        "//mcucore/src/json:json_encoder",
        "//mcucore/src/json:json_encoder_helpers",
        "//mcucore/src/log",
        "//mcucore/src/log:log_sink",
        "//mcucore/src/misc:progmem_ptr",
        "//mcucore/src/misc:to_unsigned",
        "//mcucore/src/misc:uuid",
        "//mcucore/src/platform/avr:jitter_random",
        "//mcucore/src/platform/avr:watchdog",
        "//mcucore/src/print:any_printable",
        "//mcucore/src/print:counting_print",
        "//mcucore/src/print:has_insert_into",
        "//mcucore/src/print:has_print_to",
        "//mcucore/src/print:hex_dump",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/print:print_misc",
        "//mcucore/src/print:print_to_buffer",
        "//mcucore/src/print:printable_cat",
        "//mcucore/src/print:stream_to_print",
        "//mcucore/src/semistd:limits",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/semistd:utility",
        "//mcucore/src/status",
        "//mcucore/src/status:status_code",
        "//mcucore/src/status:status_or",
        "//mcucore/src/strings:has_progmem_char_array",
        "//mcucore/src/strings:progmem_string",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_compare",
        "//mcucore/src/strings:string_view",
        "//mcucore/src/strings:tiny_string",
    ],
)

cc_library(
    name = "mcucore_config",
    srcs = ["mcucore_config.cc"],
    hdrs = ["mcucore_config.h"],
)

cc_library(
    name = "mcucore_platform",
    srcs = ["mcucore_platform.cc"],
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
