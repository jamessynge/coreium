I'd like to develop an API encapsulating access to the AVR Timer/Counter
peripherals. I have these goals:

*   Offer two interfaces to Timer/Counters:
    1.  Low-level, where the caller needs to know the different modes, order of
        access to registers, etc., but shouldn't have to know the specific
        registers or mask values; otherwise, might as well use them directly.
    1.  High-level, where the caller can make a single call to configure the
        timer/counter in some mode, possibly setting many registers.
*   No space at all if unused (including no virtual function table).
*   No or tiny permanent RAM usage;
    *   Read from registers to determine current state if needed;
    *   Ideally no virtual function table.

Template functions are likely to be quite important for the high-level API as
they would allow us to use non-type template parameters to pass values such as
the address of a register and masks. For peripherals that are identical copies
(e.g. ATmega2560's Timer/Counter 1, 3, 4 & 5), we should be able to have a class
that doesn't know the register addresses of specific instances, but is provided
with that information via some kind of parameter (template, ctor or method).
