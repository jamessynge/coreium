# Libraries specifically targetting Microchip AVR microcontrollers.

load(
    "//mcucore/extras/dev_tools:build_config.bzl",
    "build_config",
)

package(
    default_copts = build_config.default_copts,
)

cc_library(
    name = "watchdog",
    srcs = ["watchdog.cc"],
    hdrs = ["watchdog.h"],
    deps = [
        "//mcucore/extras/host/arduino:avr_wdt",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src:progmem_string_data",
    ],
)
