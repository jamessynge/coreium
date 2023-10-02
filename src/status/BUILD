# Package with support for representing the status of operations
# which might fail.

load(
    "//mcucore/extras/bazel:arduino_cc_library.bzl",
    "arduino_cc_library",
)

arduino_cc_library(
    name = "status",
    srcs = ["status.cc"],
    hdrs = ["status.h"],
    deps = [
        ":status_code",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/misc:preproc",
        "//mcucore/src/print:counting_print",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
    ],
)

arduino_cc_library(
    name = "status_code",
    srcs = ["status_code.cc"],
    hdrs = ["status_code.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/print:print_misc",
        "//mcucore/src/print:print_to_buffer",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

arduino_cc_library(
    name = "status_or",
    hdrs = ["status_or.h"],
    deps = [
        ":status",
        ":status_code",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/misc:preproc",
        "//mcucore/src/semistd:type_traits",
    ],
)
