# Syslog for McuCore

It's become clear that logging of messages over serial is convenient during
development, but once a device is deployed, it can become inconvenient or
impossible to get access to the serial output. In the context of using this for
AstroMakers devices (e.g. a Cover Calibrator at the end of a telescope, or a
rooftop weather sensor), based on an Arduino plus Power-over-Ethernet, only the
Ethernet cable is available for logging... plus of course the ASCOM Alpaca
protocol.

So, I'm exploring here the possibility of using the SYSLOG UDP protocol for
logging. More info about these protocols can be found in these RFCs: *
[RFC 5424 - The Syslog Protocol](https://www.rfc-editor.org/rfc/rfc5424) *
[RFC 5426 - Transmission of Syslog Messages over UDP](https://www.rfc-editor.org/rfc/rfc5426)
* [RFC 3164 - The BSD syslog Protocol](https://www.rfc-editor.org/rfc/rfc3164)
(for further background, but the prior two are the focus).

There is a least one Arduino Syslog library
([link](https://github.com/arcao/Syslog)), yet I'm considering writing my own so
that it works with the McuCore logging subsystem. To achieve this, I probably
want to explore ideas for new features:

1.  Allowing for multiple LogSink's to be installed at a time, so the Syslog
    feature can be part of the McuNet library.
1.  Extending McuCore's logging to support INFO, WARNING, ERROR, DFATAL and
    FATAL levels, in addition to the existing 1 through 9 VLOG levels.

## Syslog Severity

[Syslog defines 8 Severity levels](https://www.rfc-editor.org/rfc/rfc5424.html#section-6.2.1):

Number | Severity
------ | ----------------------------------------
0      | Emergency: system is unusable
1      | Alert: action must be taken immediately
2      | Critical: critical conditions
3      | Error: error conditions
4      | Warning: warning conditions
5      | Notice: normal but significant condition
6      | Informational: informational messages
7      | Debug: debug-level messages

It is probably best to switch some MCU_VLOG messages (e.g. those produced when
first booting, which announce the software version) to the new MCU_LOG(INFO)
feature. Such messages are expected to always be compiled into the program,
though that decision could also be handled by mapping the named severities
(INFO, WARNING, ERROR, FATAL) to numeric values (e.g. 0, -1, -2, -3).
Furthermore, we can separate the decision of whether to compile a message into
the program from the runtime decision as to whether the message is active; i.e.
the latter can be done by essentially treating MCU_LOG(severity) as:

```
MCU_LOG_IF(severity, MCU_LOG_IS_ON(severity))
```

## Syslog Facility

[Syslog documents 24 facility values](https://www.rfc-editor.org/rfc/rfc5424.html#section-6.2.1):

Number | Facility
------ | ----------------------------------------
0      | kernel messages
1      | user-level messages
2      | mail system
3      | system daemons
4      | security/authorization messages
5      | messages generated internally by syslogd
6      | line printer subsystem
7      | network news subsystem
8      | UUCP subsystem
9      | clock daemon
10     | security/authorization messages
11     | FTP daemon
12     | NTP subsystem
13     | log audit
14     | log alert
15     | clock daemon (note 2)
16     | local use 0 (local0)
17     | local use 1 (local1)
18     | local use 2 (local2)
19     | local use 3 (local3)
20     | local use 4 (local4)
21     | local use 5 (local5)
22     | local use 6 (local6)
23     | local use 7 (local7)

It isn't obvious that these map well to the embedded world, though I expect we
could use 0 or 3 for McuCore and McuNet messages; 1 or 16 to 23 could be used
for the TinyAlpacaServer messages, and 10 could possibly be used for messages
related bad HTTP requests. The simplest solution is to just choose one of the
local use values as the facility value used for all messages, maybe via a C++
preprocessor macro so that each file can override the facility if appropriate.

## Misc. Notes

*   Most Arduinos, and similar embedded devices, don't know the current time or
    their own hostname, so must emit what RFC 5424 calls the `NILVALUE` (a
    single hyphen, or minus, ASCII code 45 decimal) in their place. *It is
    possible that the Ethernet library can get a hostname via DHCP; to be
    studied.*

*   Some messages will need to be logged before the network is up and running.
    One way to deal with that is to just log them to serial. On systems with
    some more memory, another option is to include a buffering feature, where
    either the initial messages can be buffered, or just the last N bytes worth
    of messages are kept until a log sink is added.

*   Regarding sending log messages to multiple log sinks, one choice is to
    change the logging macros so that rather than instantiating a LogSink, they
    instead instantiate a LogMessage, which takes care of collecting the
    necessary info for any kind of log sink, and forwarding that info to the
    registered set of sinks. This might then enable a tree of log sinks; for
    example, the root could be a ring buffer of recent messages, followed by a
    set of other log sinks; adding a log sink to the set could have the effect
    of triggering it to emit the buffered messages.

*   PROCID (process id) is a number used to distinguish messages from different
    runs of a program, or from different programs; the same value appears in all
    messages from that process. In our case, we can generate a random number at
    boot time and use that as the PROCID for all Syslog messages.
