# Contains tests of code that is in the src directory, to be executed on host.
# We don't put the tests in the src directory because otherwise the Arduino IDE
# treats them as code to be compiled for the target microcontroller, which fails.

cc_test(
    name = "mcucore_config_test",
    srcs = ["mcucore_config_test.cc"],
    deps = [
        "//glog:logging",
        "//googletest:gunit_main",
        "//mcucore/src:mcucore_config",
        "//mcucore/src:mcucore_platform",
    ],
)
