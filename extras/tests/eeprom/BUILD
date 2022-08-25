# TODO(jamessynge): describe this package.

cc_test(
    name = "eeprom_io_test",
    srcs = ["eeprom_io_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/host/eeprom",
        "//mcucore/extras/test_tools:eeprom_test_utils",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/src/eeprom:eeprom_io",
        "//mcucore/src/hash:crc32",
        "//mcucore/src/strings:progmem_string_view",
    ],
)

cc_test(
    name = "eeprom_region_test",
    srcs = ["eeprom_region_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/host/eeprom",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:status_or_test_utils",
        "//mcucore/src/eeprom:eeprom_region",
        "//mcucore/src/status:status_code",
        "//mcucore/src/strings:string_view",
    ],
)

cc_test(
    name = "eeprom_tag_test",
    srcs = ["eeprom_tag_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:eeprom_example_domains",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/src/eeprom:eeprom_tag",
    ],
)

cc_test(
    name = "eeprom_tlv_test",
    srcs = ["eeprom_tlv_test.cc"],
    deps = [
        "//absl/log",
        "//absl/strings",
        "//googletest:gunit_main",
        "//mcucore/extras/host/eeprom",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:status_or_test_utils",
        "//mcucore/extras/test_tools:status_test_utils",
        "//mcucore/src:mcucore_platform",
        "//mcucore/src/eeprom:eeprom_region",
        "//mcucore/src/eeprom:eeprom_tag",
        "//mcucore/src/eeprom:eeprom_tlv",
        "//mcucore/src/status",
        "//mcucore/src/status:status_or",
        "//mcucore/src/strings:string_view",
    ],
)
