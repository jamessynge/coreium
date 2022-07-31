# Implementations of some C++ Standard Library features for the
# Arduino / avr-libc environment, which doesn't include those libraries;
# avr-libc primarily has a C libraries.

load(
    "//mcucore/extras/dev_tools:build_config.bzl",
    "build_config",
)

package(
    default_copts = build_config.default_copts,
)

cc_library(
    name = "utility",
    hdrs = ["utility.h"],
    deps = [
        "//mcucore/src:mcucore_platform",
        "//mcucore/src:type_traits",
    ],
)
