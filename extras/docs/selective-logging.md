# Selective DCHECK and VLOG

## Falling back to MCU_VLOG_IF

When MCU_DCHECK is disabled, it would be nice to have the option of treating
some of those as MCU_VLOG_IF instead (i.e. don't fail, but invoke MCU_VLOG if
the condition is not met).

```c++
#ifdef MCU_ENABLE_DCHECK

#define MCU_DVCHECK(level, expression) MCU_DCHECK(expression)
#define MCU_DVCHECK_EQ(level, a, b) MCU_DCHECK_EQ(a, b)
#define MCU_DVCHECK_NE(level, a, b) MCU_DCHECK_NE(a, b)
#define MCU_DVCHECK_LT(level, a, b) MCU_DCHECK_LT(a, b)
#define MCU_DVCHECK_LE(level, a, b) MCU_DCHECK_LE(a, b)
#define MCU_DVCHECK_GE(level, a, b) MCU_DCHECK_GE(a, b)
#define MCU_DVCHECK_GT(level, a, b) MCU_DCHECK_GT(a, b)

#else

#define MCU_DVCHECK(level, expression) MCU_VLOG_IF(level, expression)
#define MCU_DVCHECK_EQ(level, a, b) MCU_VLOG_IF_EQ(level, a, b)
#define MCU_DVCHECK_NE(level, a, b) MCU_VLOG_IF_NE(level, a, b)
#define MCU_DVCHECK_LT(level, a, b) MCU_VLOG_IF_LT(level, a, b)
#define MCU_DVCHECK_LE(level, a, b) MCU_VLOG_IF_LE(level, a, b)
#define MCU_DVCHECK_GE(level, a, b) MCU_VLOG_IF_GE(level, a, b)
#define MCU_DVCHECK_GT(level, a, b) MCU_VLOG_IF_GT(level, a, b)

#endif
```

## Selective logging

As of July, 2022, McuCore supports compile time selection of which `MCU_VLOG`
statements are active based on the level value passed in to `MCU_VLOG` and the
value of the macro `MCU_ENABLED_VLOG_LEVEL` (e.g. 3). With rare exceptions, all
files share the same value of `MCU_ENABLED_VLOG_LEVEL`, which is specified in
`mcucore_config.h`. However, I'd like to make two improvements to this:

*   Make it easy to override `MCU_ENABLED_VLOG_LEVEL` within individual files.
    This isn't currently possible because of the way that `logging.h` defines a
    different version of `MCU_VLOG` depending on the value of
    `MCU_ENABLED_VLOG_LEVEL` when it is evaluated by the preprocessor.
*   Provide the option to enable or disable logging messages at runtime, as is
    possible with [glog](https://github.com/google/glog). Specifically, I'd like
    the option to set the enabled log level on a per file, per library or per
    sketch level, with the more specific value taking precedence.

> NOTE: I didn't propose per directory control of logging because that amps up
> the difficulty of implementation considerably. In particular, my idea of how
> to implement this is based on making use of `MCU_BASENAME`, which doesn't
> (currently) have support for generating a type representing a file path.
>
> It happens that providing per library control may amount to the same thing as
> providing per directory, so I'm likely to start with per file and per sketch
> as the only two dynamically modifiable logging levels.

### Implementation idea

`MCU_PSD(str)` (defined in progmem_string_data.h) produces a type (approximately
`ProgmemStringData<str>`) that encapsulates the string `str`.
`MCU_BASENAME(__FILE__)` produces a type that does the same for the basename
portion of `__FILE__`. Given those, we can define a class or function template
whose type parameter is produced by the same macros as used by `MCU_BASENAME`,
and provides access to variable storage used to determine the enabled logging
level. For example, in the definition of MCU_VLOG, we might have the following:

```c++
#define MCU_VLOG(level)                          \
  switch (0)                                     \
  default:                                       \
    (!MCU_VLOG_IS_ON_FOR_FILE(level, __FILE__))  \
        ? (void)0                                \
        : ::mcucore::LogSinkVoidify() &&         \
              ::mcucore::LogSink(MCU_VLOG_LOCATION(__FILE__), __LINE__)
```

We could then implement `MCU_VLOG_IS_ON_FOR_FILE` in part using a function
template somewhat like this:

```c++
namespace mcucore {
template <typename FILE_PSD>
bool IsVlogEnabled() {
  static VlogEnableData vlog_enable_data(FILE_PSD::kData, FILE_PSD::size());
  return vlog_enable_data.enabled();
}
}  // namespace mcucore
```

One approach to linking all of the VlogEnableData instances (or similar)
together is to use linker sections. For example, we might have the immutable
data (e.g. the name of the file and the address of the mutable level variable),
stored in PROGMEM in a custom linker section. The linker will combine all such
entries into a single table, with generated symbols that mark the start and end
of the table (so that our code can find the bounds of the table). At boot time,
and any time later when the logging level is dynamically changed, we can use
that table to find the affected entries and update them.

References:

*   https://stackoverflow.com/questions/39733931/how-to-implement-a-compile-time-dispatch-table-for-avr

*   https://stackoverflow.com/questions/11840651/how-can-i-implement-a-dynamic-dispatch-table-in-c/32192330#32192330

*   https://www.google.com/search?q=g%2B%2B+create+table+using+linker+sections+at+compile+time

If we need to change the args to the compiler, Arduino provides a mechanism via
the file
[platform.local.txt](https://arduino.github.io/arduino-cli/0.19/platform-specification/#platformlocaltxt).

### Selective logging questions

Assuming this feature is developed...

#### Is it always enabled, regardless of level?

It will take storage to keep track of which files are enabled. For example, if
we have a single byte per source file for the enabled level, and 2 bytes for a
pointer linking (on the ATmega2560)

*   It could be that it is enabled on an approximately per source file basis.
    For example, we could have a macro `MCU_DYNAMICALLY_ENABLED_VLOG_LEVEL`,
    which if defined to a value from 1 to 9, specifies that values at or lower
    than the specified value are enabled dynamically.

    *   This macro could be defined in a header such as McuCore's
        mcucore_config.h, and thus apply to the entire sketch, or could be
        defined at the top of a `.cc` file, before McuCore's logging.h is
        included.

*   One idea is to provide two compile time controls of enabled logging levels,
    one specifying a level that is turned on when the sketch starts up, and
    another that can be turned on after the fact. E.g. levels 1 and 2 are on by
    default, levels 3 through 5 can be turned on dynamically, and levels 6
    through 9 are disabled at compile time.

*   Another idea is to store data in EEPROM (using `EepromTlv`) deciding whether
    the logging for a particular file is enabled. But, given the way that
    EepromTlv works, what EepromTags would we use for this data? Perhaps a
    single tag, or a single domain with a randomly selected set of EepromTag
    ids?
