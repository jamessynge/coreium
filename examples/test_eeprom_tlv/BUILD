load(
    "//mcucore/extras/bazel:arduino_cc_sketch.bzl",
    "arduino_cc_sketch",
)

arduino_cc_sketch(
    name = "test_eeprom_tlv",
    srcs = ["test_eeprom_tlv.ino.cc"],
    deps = [
        "//mcucore/extras/host/arduino",
        "//mcucore/extras/host/arduino:arduino_main",
        "//mcucore/src:McuCore",
    ],
)
