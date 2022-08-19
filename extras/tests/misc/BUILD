# TODO(jamessynge): describe this package.

cc_test(
    name = "to_unsigned_test",
    srcs = ["to_unsigned_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src/misc:to_unsigned",
        "//mcucore/src/semistd:type_traits",
    ],
)

cc_test(
    name = "uuid_test",
    srcs = ["uuid_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/host/eeprom",
        "//mcucore/extras/test_tools:print_value_to_std_string",
        "//mcucore/extras/test_tools:status_or_test_utils",
        "//mcucore/extras/test_tools:status_test_utils",
        "//mcucore/src/eeprom:eeprom_tag",
        "//mcucore/src/eeprom:eeprom_tlv",
        "//mcucore/src/misc:uuid",
        "//mcucore/src/status:status_code",
    ],
)
