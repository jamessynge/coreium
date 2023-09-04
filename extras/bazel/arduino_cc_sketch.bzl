"""Rule for defining an Arduino sketch target."""

load(
    "//devtools/build_cleaner/skylark:build_defs.bzl",
    "register_extension_info",
)
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

# Tell build_cleaner about arduino_cc_sketch. In particular,
# when build_cleaner determines that it needs to update the deps of:
#
#     cc_binary(foo_sketch)
#
# which doesn't exist in the BUILD file, it will instead update:
#
#     arduino_cc_sketch(foo_sketch)
#
register_extension_info(
    extension = arduino_cc_sketch,
    label_regex_for_dep = "{extension_name}",
)
