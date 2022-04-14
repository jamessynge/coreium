load(
    "//mcucore/extras/dev_tools:build_config.bzl",
    "build_config",
)

package(
    default_copts = build_config.default_copts,
)

cc_binary(
    name = "eeprom_tlv_ino",
    srcs = ["eeprom_tlv.ino.cc"],
    deps = [
        "//mcucore/examples/eeprom_tlv/src:example1",
        "//mcucore/examples/eeprom_tlv/src:example2",
        "//mcucore/extras/host/arduino:arduino_main",
        "//mcucore/src:McuCore",
    ],
)
