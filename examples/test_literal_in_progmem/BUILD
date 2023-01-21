load(
    "//mcucore/extras/bazel:arduino_cc_sketch.bzl",
    "arduino_cc_sketch",
)

arduino_cc_sketch(
    name = "test_literal_in_progmem",
    srcs = ["test_literal_in_progmem.ino.cc"],
    deps = [
        "//mcucore/extras/host/arduino",
        "//mcucore/extras/host/arduino:arduino_main",
        "//mcucore/src:McuCore",
    ],
)
