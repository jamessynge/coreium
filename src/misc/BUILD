# Miscellaneous libraries that don't fit into other categories.

load(
    "//mcucore/extras/bazel:arduino_cc_library.bzl",
    "arduino_cc_library",
)

arduino_cc_library(
    name = "preproc",
    hdrs = ["preproc.h"],
)

arduino_cc_library(
    name = "progmem_ptr",
    hdrs = ["progmem_ptr.h"],
    deps = ["//mcucore/src:mcucore_platform"],
)

arduino_cc_library(
    name = "to_unsigned",
    hdrs = ["to_unsigned.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/semistd:type_traits",
    ],
)

arduino_cc_library(
    name = "uuid",
    srcs = ["uuid.cc"],
    hdrs = ["uuid.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/eeprom:eeprom_tag",
        "//mcucore/src/eeprom:eeprom_tlv",
        "//mcucore/src/log",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/status",
    ],
)
