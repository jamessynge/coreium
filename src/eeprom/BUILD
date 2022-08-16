# TODO(jamessynge): describe this package.

cc_library(
    name = "eeprom_io",
    srcs = ["eeprom_io.cc"],
    hdrs = ["eeprom_io.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/hash:crc32",
        "//mcucore/src/log",
        "//mcucore/src/strings:progmem_string_view",
    ],
)

cc_library(
    name = "eeprom_region",
    srcs = ["eeprom_region.cc"],
    hdrs = ["eeprom_region.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/container:flash_string_table",
        "//mcucore/src/log",
        "//mcucore/src/semistd:limits",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/status:status_or",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:string_view",
    ],
)

cc_library(
    name = "eeprom_tag",
    srcs = ["eeprom_tag.cc"],
    hdrs = ["eeprom_tag.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
    ],
)

cc_library(
    name = "eeprom_tlv",
    srcs = ["eeprom_tlv.cc"],
    hdrs = ["eeprom_tlv.h"],
    deps = [
        ":eeprom_region",
        ":eeprom_tag",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/hash:crc32",
        "//mcucore/src/log",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/semistd:limits",
        "//mcucore/src/status",
        "//mcucore/src/status:status_code",
        "//mcucore/src/status:status_or",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_compare",
        "//mcucore/src/strings:string_view",
    ],
)
