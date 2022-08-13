# Contains tests of code that is in the mcucore/src/semistd directory, to be
# executed on host, not on the embedded target. We don't put the tests in the
# src directory because otherwise the Arduino IDE treats them as code to be
# compiled for the target microcontroller, which fails.

cc_test(
    name = "utility_test",
    srcs = ["utility_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/extras/test_tools:pretty_type_name",
        "//mcucore/src/semistd:utility",
    ],
)

cc_test(
    name = "limits_test",
    srcs = ["limits_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src/semistd:limits",
    ],
)

cc_test(
    name = "type_traits_test",
    srcs = ["type_traits_test.cc"],
    deps = [
        "//googletest:gunit_main",
        "//mcucore/src/semistd:type_traits",
    ],
)
