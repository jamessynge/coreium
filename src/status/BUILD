# TODO(jamessynge): describe this package.

cc_library(
    name = "status",
    srcs = ["status.cc"],
    hdrs = ["status.h"],
    deps = [
        ":status_code",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/print:counting_print",
        "//mcucore/src/print:hex_escape",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/strings:progmem_string_data",
        "//mcucore/src/strings:progmem_string_view",
    ],
)

cc_library(
    name = "status_code",
    srcs = ["status_code.cc"],
    hdrs = ["status_code.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/container:flash_string_table",
        "//mcucore/src/print:print_misc",
        "//mcucore/src/semistd:type_traits",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

cc_library(
    name = "status_or",
    hdrs = ["status_or.h"],
    deps = [
        ":status",
        ":status_code",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/log",
        "//mcucore/src/semistd:type_traits",
    ],
)
