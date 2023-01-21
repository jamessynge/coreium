load(
    "//mcucore/extras/bazel:arduino_cc_sketch.bzl",
    "arduino_cc_sketch",
)

arduino_cc_sketch(
    name = "eeprom_tlv_ino",
    srcs = ["eeprom_tlv.ino.cc"],
    deps = [
        "//mcucore/examples/eeprom_tlv/src:example1",
        "//mcucore/examples/eeprom_tlv/src:example2",
        "//mcucore/extras/host/arduino:arduino_main",
        "//mcucore/src:McuCore",
    ],
)
