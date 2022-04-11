load(
    "//mcucore/extras/dev_tools:build_config.bzl",
    "build_config",
)

package(
    default_copts = build_config.default_copts,
)

cc_binary(
    name = "study_watchdog",
    srcs = ["study_watchdog.ino.cc"],
    deps = [
        "//mcucore/extras/host/arduino:arduino_main",
        "//mcucore/src:McuCore",
    ],
)
