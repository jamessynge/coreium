# Two source files, each defining separate EEPROM domains.

load(
    "//mcucore/extras/bazel:arduino_cc_library.bzl",
    "arduino_cc_library",
)

arduino_cc_library(
    name = "example1",
    srcs = ["example1.cc"],
    hdrs = ["example1.h"],
    deps = ["//mcucore/src:McuCore"],
)

arduino_cc_library(
    name = "example2",
    srcs = ["example2.cc"],
    hdrs = ["example2.h"],
    deps = ["//mcucore/src:McuCore"],
)
