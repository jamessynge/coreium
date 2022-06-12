# Host tools to aid development.

load("//devtools/python/blaze:pytype.bzl", "pytype_strict_binary", "pytype_strict_library")
load("//tools/build_defs/testing:bzl_library.bzl", "bzl_library")

bzl_library(
    name = "build_config_bzl",
    srcs = ["build_config.bzl"],
    parse_tests = False,
    visibility = ["//visibility:private"],
)

pytype_strict_binary(
    name = "capture_serial_port",
    srcs = ["capture_serial_port.py"],
    deps = ["//third_party/py/serial"],
)

cc_binary(
    name = "crc32_table_generator",
    srcs = ["crc32_table_generator.cc"],
)

pytype_strict_library(
    name = "make_keep_literal",
    srcs = ["make_keep_literal.py"],
)
