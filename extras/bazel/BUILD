load("//tools/build_defs/testing:bzl_library.bzl", "bzl_library")

bzl_library(
    name = "arduino_build_config_bzl",
    srcs = ["arduino_build_config.bzl"],
)

bzl_library(
    name = "arduino_cc_library_bzl",
    srcs = ["arduino_cc_library.bzl"],
    deps = [
        ":arduino_build_config_bzl",
        "//devtools/build_cleaner/skylark:build_defs_lib",
    ],
)

bzl_library(
    name = "arduino_cc_sketch_bzl",
    srcs = ["arduino_cc_sketch.bzl"],
    deps = [
        ":arduino_build_config_bzl",
        "//devtools/build_cleaner/skylark:build_defs_lib",
    ],
)
