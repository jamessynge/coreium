"""Rule for defining an Arduino library target."""

load(":arduino_build_config.bzl", "arduino_copts")

def arduino_cc_library(
        name,
        srcs = [],
        hdrs = [],
        copts = [],
        visibility = None,
        tags = [],
        deps = [],
        strip_include_prefix = None,
        include_prefix = None,
        textual_hdrs = None,
        alwayslink = None,
        defines = []):
    """C++ libraries targetting Arduino should be specified with this function."""
    native.cc_library(
        name = name,
        srcs = srcs,
        hdrs = hdrs,
        copts = arduino_copts() + copts,
        visibility = visibility,
        tags = tags,
        textual_hdrs = textual_hdrs,
        deps = deps,
        alwayslink = alwayslink,
        strip_include_prefix = strip_include_prefix,
        include_prefix = include_prefix,
        defines = defines,
    )
