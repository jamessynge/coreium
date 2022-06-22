# About McuCore

C++ core utilities for Arduino sketches, and for testing such code on host.

## About extras/host/...

I find that I can develop software (think, code, compile/link, test/debug,
think) much faster by iterating on a host (Linux box, in my case), rather than
re-uploading to the target Arduino for each test/debug phase. Therefore I've
recreated (including copying of APIs) just enough of the Arduino API, and
relevant libraries (e.g. EEPROM), to enable this.

I've written most of my tests based on googletest, a C++ Unit Testing framework
from Google. I use it at work and really appreciate what it provides.

*   https://github.com/google/googletest/

### Other On-Host Testing Solutions for Arduino

Others have explored the topic of host based unit testing of Arduino libraries,
and also supporting embedded unit testing of Arduino code:

*   https://github.com/bxparks/EpoxyDuino
*   https://github.com/mmurdoch/arduinounit
*   https://github.com/bxparks/AUnit

## Planning

*   MAYBE: Support copying literal (e.g. PROGMEM) strings into stack variables
    for temporary needs. For example, use a template class with a field of type
    char[N], where N is the length of the literal (without a terminating NUL),
    copy the literal into the field at construction time, and provide the
    ability to construct a mcucore::StringView from it.

    *   This could be implemented with a TinyString<T> ctor or member. Today
        this can be achieved using PrintToBuffer writing to a TinyString.

*   Add support for reading files from the SDcard, and maybe writing them. I'm
    planning to use Bill Greiman's SdFat library which supports long file names
    (127 ASCII characters). Non-normalized paths should be rejected (e.g. those
    with `//`, `/../`, or `/./`).
