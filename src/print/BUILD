# Provides support for printing and formatting. In particular, provides services
# on top of Arduino's Print class, such as OPrintStream, PrintToBuffer, and
# HexEscapedPrintable.

cc_library(
    name = "any_printable",
    srcs = ["any_printable.cc"],
    hdrs = ["any_printable.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/strings:progmem_string",
        "//mcucore/src/strings:progmem_string_view",
        "//mcucore/src/strings:string_view",
        "//mcucore/src/strings:tiny_string",
    ],
)

cc_library(
    name = "counting_print",
    srcs = ["counting_print.cc"],
    hdrs = ["counting_print.h"],
    deps = ["//mcucore/src:mcucore_platform"],
)

cc_library(
    name = "has_insert_into",
    hdrs = ["has_insert_into.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/semistd:type_traits",
    ],
)

cc_library(
    name = "has_print_to",
    hdrs = ["has_print_to.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/semistd:type_traits",
    ],
)

cc_library(
    name = "hex_dump",
    srcs = ["hex_dump.cc"],
    hdrs = ["hex_dump.h"],
    deps = [
        ":hex_escape",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/semistd:limits",
        "//mcucore/src/strings:progmem_string_view",
    ],
)

cc_library(
    name = "hex_escape",
    srcs = ["hex_escape.cc"],
    hdrs = ["hex_escape.h"],
    deps = [
        ":counting_print",
        ":has_print_to",
        ":o_print_stream",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/semistd:type_traits",
    ],
)

cc_library(
    name = "o_print_stream",
    hdrs = ["o_print_stream.h"],
    deps = [
        ":has_insert_into",
        ":has_print_to",
        ":print_misc",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/misc:to_unsigned",
        "//mcucore/src/semistd:limits",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/strings:has_progmem_char_array",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

cc_library(
    name = "print_misc",
    srcs = ["print_misc.cc"],
    hdrs = ["print_misc.h"],
    deps = [
        ":counting_print",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/strings:has_progmem_char_array",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

cc_library(
    name = "print_to_buffer",
    srcs = ["print_to_buffer.cc"],
    hdrs = ["print_to_buffer.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/strings:string_view",
        "//mcucore/src/strings:tiny_string",
    ],
)

cc_library(
    name = "printable_cat",
    srcs = ["printable_cat.cc"],
    hdrs = ["printable_cat.h"],
    deps = [
        ":any_printable",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/container:array",
        "//mcucore/src/container:array_view",
    ],
)

cc_library(
    name = "stream_to_print",
    hdrs = ["stream_to_print.h"],
    deps = [
        ":o_print_stream",
        "//mcucore/src:mcucore_platform",
    ],
)
