cc_library(
    name = "tiny_string",
    hdrs = ["tiny_string.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
    ],
)

cc_library(
    name = "string_view",
    srcs = ["string_view.cc"],
    hdrs = ["string_view.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/print:has_print_to",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/print:o_print_stream",
    ],
)

cc_library(
    name = "string_compare",
    srcs = ["string_compare.cc"],
    hdrs = ["string_compare.h"],
    deps = [
        ":progmem_string_view",
        ":string_view",
        "//mcucore/src:mcucore_platform",
    ],
)

cc_library(
    name = "progmem_string_view",
    srcs = ["progmem_string_view.cc"],
    hdrs = ["progmem_string_view.h"],
    deps = [
        ":has_progmem_char_array",
        ":progmem_string_data",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/misc:progmem_ptr",
        "//mcucore/src/print:has_print_to",
        "//mcucore/src/print:print_misc",
        "//mcucore/src/semistd:type_traits",
    ],
)

cc_library(
    name = "progmem_string_data",
    hdrs = ["progmem_string_data.h"],
    deps = ["//mcucore/src:mcucore_platform"],
)

cc_library(
    name = "progmem_string",
    srcs = ["progmem_string.cc"],
    hdrs = ["progmem_string.h"],
    deps = [
        ":has_progmem_char_array",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/semistd:type_traits",
    ],
)

cc_library(
    name = "has_progmem_char_array",
    hdrs = ["has_progmem_char_array.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/semistd:type_traits",
    ],
)
