load(
    "//mcucore/extras/bazel:arduino_cc_sketch.bzl",
    "arduino_cc_sketch",
)

arduino_cc_sketch(
    name = "jitter_random_exercise",
    srcs = ["jitter_random_exercise.ino.cc"],
    deps = [
        "//mcucore/extras/host/arduino",
        "//mcucore/extras/host/arduino:arduino_main",
        "//mcucore/src:McuCore",
    ],
)
