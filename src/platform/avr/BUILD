# Libraries specifically targetting Microchip AVR microcontrollers.

load(
    "//mcucore/extras/bazel:arduino_cc_library.bzl",
    "arduino_cc_library",
)

arduino_cc_library(
    name = "jitter_random",
    srcs = ["jitter_random.cc"],
    hdrs = ["jitter_random.h"],
    deps = [
        ":watchdog",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/hash:crc32",
        "//mcucore/src/hash:fnv1a",
        "//mcucore/src/log",
        "//mcucore/src/print:o_print_stream",
        "//mcucore/src/semistd:limits",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

arduino_cc_library(
    name = "timer_counter",
    srcs = ["timer_counter.cc"],
    hdrs = ["timer_counter.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/container:flash_string_table",
        "//mcucore/src/log",
        "//mcucore/src/print:counting_print",
        "//mcucore/src/print:has_print_to",
        "//mcucore/src/print:print_misc",
        "//mcucore/src/print:print_to_buffer",
        "//mcucore/src/print:stream_to_print",
        "//mcucore/src/strings:progmem_string_data",
    ],
)

arduino_cc_library(
    name = "watchdog",
    srcs = ["watchdog.cc"],
    hdrs = ["watchdog.h"],
    deps = [
        "//mcucore/extras/host/arduino:avr_wdt",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/strings:progmem_string_data",
    ],
)
