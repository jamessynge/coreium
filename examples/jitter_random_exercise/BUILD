load(
    "//mcucore/extras/dev_tools:build_config.bzl",
    "build_config",
)

package(
    default_copts = build_config.default_copts,
)

cc_binary(
    name = "jitter_random_exercise",
    srcs = ["jitter_random_exercise.ino.cc"],
    deps = [
        "//mcucore/extras/host/arduino",
        "//mcucore/extras/host/arduino:arduino_main",
        "//mcucore/src:McuCore",
        "//mcucore/src:jitter_random",
        "//mcucore/src:log_sink",
        "//mcucore/src:o_print_stream",
        "//mcucore/src:progmem_string",
    ],
)
