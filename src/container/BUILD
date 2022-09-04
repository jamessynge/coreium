# Container classes.

cc_library(
    name = "array",
    hdrs = ["array.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
    ],
)

cc_library(
    name = "array_view",
    hdrs = ["array_view.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
    ],
)

cc_library(
    name = "flash_string_table",
    srcs = ["flash_string_table.cc"],
    hdrs = ["flash_string_table.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/strings:has_progmem_char_array",
    ],
)

cc_library(
    name = "serial_map",
    hdrs = ["serial_map.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/semistd:limits",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/status",
        "//mcucore/src/status:status_or",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:string_view",
    ],
)
