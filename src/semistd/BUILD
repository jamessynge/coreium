# Implementations of some C++ Standard Library features for the
# Arduino / avr-libc environment, which doesn't include those libraries;
# avr-libc primarily has a C libraries.

load(
    "//mcucore/extras/bazel:arduino_cc_library.bzl",
    "arduino_cc_library",
)

arduino_cc_library(
    name = "limits",
    hdrs = ["limits.h"],
    deps = [
        ":type_traits",
        "//mcucore/src:mcucore_platform",
    ],
)

arduino_cc_library(
    name = "type_traits",
    hdrs = ["type_traits.h"],
    deps = ["//mcucore/src:mcucore_platform"],
)

arduino_cc_library(
    name = "utility",
    hdrs = ["utility.h"],
    deps = [
        ":type_traits",
        "//mcucore/src:mcucore_platform",
    ],
)
