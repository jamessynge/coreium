"""Rule for defining an Arduino sketch target."""

load(":arduino_build_config.bzl", "arduino_copts")

def arduino_cc_sketch(
        name,
        srcs = [],  # Note: could compute this from the name.
        copts = [],
        data = [],
        visibility = None,
        deps = [],
        linkopts = [],
        tags = [],
        features = []):
    """Arduino Sketch targets should be specified with this function."""
    native.cc_binary(
        name = name,
        srcs = srcs,
        copts = arduino_copts() + copts,
        data = data,
        visibility = visibility,
        deps = deps,
        linkopts = linkopts,
        linkstatic = 1,
        tags = tags,
        features = features,
    )
