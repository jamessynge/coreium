# This package contains the subset of what Arduino.h offers and is used by
# Tiny Alpaca Server, thus allowing us to build and test the server on a host,
# instead of on an Arduino.
#
# In addition, this package helps us work around the fact that Arduino doesn't
# have the Standard Template Library, even those methods that don't require
# memory allocation, such as std::numeric_limits<int>::max(). Therefore we
# provide libraries here that expose the relevant functions to the rest of
# Tiny Alpaca Server, and do NOT require importing STL headers in most source
# files.

cc_library(
    name = "arduino",
    srcs = ["arduino.cc"],
    hdrs = ["arduino.h"],
    deps = [
        ":avr_io",
        ":pgmspace",
        ":print",
        ":random",
        ":serial",
        ":stream",
        ":wcharacter",
        ":wstring",
        "//absl/time",
    ],
)

cc_library(
    name = "arduino_main",
    srcs = ["arduino_main.cc"],
    linkopts = [
        # Don't complain that setup and loop aren't defined. They'll be provided
        # by the cc_binary that links in this library, by way of the setup and
        # loop functions in the source of the sketch (.ino file).
        "-Wl,--undefined=setup",
        "-Wl,--undefined=loop",
    ],
    deps = [
        ":call_setup_and_loop",
        "//base",
    ],
)

cc_library(
    name = "avr_io",
    srcs = ["avr_io.cc"],
    hdrs = ["avr_io.h"],
)

cc_library(
    name = "avr_wdt",
    srcs = ["avr_wdt.cc"],
    hdrs = ["avr_wdt.h"],
)

cc_library(
    name = "call_setup_and_loop",
    srcs = ["call_setup_and_loop.cc"],
    hdrs = ["call_setup_and_loop.h"],
    linkopts = [
        # Don't complain that setup and loop aren't defined. They'll be provided
        # by the cc_binary that links in this library, by way of the setup and
        # loop functions in the source of the sketch (.ino file).
        "-Wl,--undefined=setup",
        "-Wl,--undefined=loop",
    ],
    deps = [
        "//absl/flags:flag",
        "//absl/log",
        "//absl/log:check",
        "//absl/time",
    ],
)

cc_library(
    name = "pgmspace",
    srcs = ["pgmspace.cc"],
    hdrs = ["pgmspace.h"],
)

cc_library(
    name = "print",
    srcs = ["print.cc"],
    hdrs = ["print.h"],
    deps = [
        ":printable",
        ":wstring",
        "//absl/log:check",
    ],
)

cc_library(
    name = "random",
    srcs = ["random.cc"],
    hdrs = ["random.h"],
    deps = [
        "//absl/random",
        "//absl/random:distributions",
        "//absl/time",
    ],
)

cc_library(
    name = "serial",
    srcs = ["serial.cc"],
    hdrs = ["serial.h"],
    deps = [":print"],
)

cc_library(
    name = "stream",
    hdrs = ["stream.h"],
    deps = [":print"],
)

cc_library(
    name = "udp",
    hdrs = ["udp.h"],
    deps = [
        ":ip_address",
        ":stream",
    ],
)

cc_library(
    name = "wcharacter",
    srcs = ["wcharacter.cc"],
    hdrs = ["wcharacter.h"],
    deps = ["//absl/strings"],
)

cc_library(
    name = "wstring",
    hdrs = ["wstring.h"],
)

cc_library(
    name = "client",
    hdrs = ["client.h"],
    deps = [
        ":ip_address",
        "//mcucore/extras/host/arduino:stream",
    ],
)

cc_library(
    name = "ip_address",
    hdrs = ["ip_address.h"],
    deps = [
        "//mcucore/extras/host/arduino:print",
    ],
)

cc_library(
    name = "printable",
    hdrs = ["printable.h"],
)
