#ifndef MCUCORE_EXTRAS_HOST_ARDUINO_AVR_IO_H_
#define MCUCORE_EXTRAS_HOST_ARDUINO_AVR_IO_H_

#include <stdint.h>

#ifndef AVR_IO_REGISTER_LINKAGE
#define AVR_IO_REGISTER_LINKAGE extern
#endif  // !AVR_IO_REGISTER_LINKAGE

#ifndef AVR_IO_CONST
#define AVR_IO_CONST(type, name, value) \
  constexpr type name = value;          \
  static_assert(name == value, "dummy use of the variable")
#endif  // !AVR_IO_CONST

#define DEFINE_AVR_8_BIT_TIMER_COUNTER_REGISTERS_AND_FIELDS(n)     \
  /* T/C Control Registers A and B. */                             \
  AVR_IO_REGISTER_LINKAGE volatile uint8_t TCCR##n##A, TCCR##n##B; \
  /* T/C Output Compare Registers A and B. */                      \
  AVR_IO_REGISTER_LINKAGE volatile uint8_t OCR##n##A, OCR##n##B;   \
  /* T/C Interrupt Mask and Flag Registers. */                     \
  AVR_IO_REGISTER_LINKAGE volatile uint8_t TIMSK##n, TIFR##n;      \
  /* T/C Counter Register. */                                      \
  AVR_IO_REGISTER_LINKAGE volatile uint8_t TCNT##n;                \
  /* Compare Match Output Mode bit fields, in TCCRnA. */           \
  AVR_IO_CONST(uint8_t, COM##n##A1, 7);                            \
  AVR_IO_CONST(uint8_t, COM##n##A0, 6);                            \
  AVR_IO_CONST(uint8_t, COM##n##B1, 5);                            \
  AVR_IO_CONST(uint8_t, COM##n##B0, 4);                            \
  /* Waveform Generation bit fields, in TCCRnA and TCCRnB. */      \
  AVR_IO_CONST(uint8_t, WGM##n##2, 3); /* TCCRnB */                \
  AVR_IO_CONST(uint8_t, WGM##n##1, 1); /* TCCRnA */                \
  AVR_IO_CONST(uint8_t, WGM##n##0, 0); /* TCCRnA */                \
  /* Clock select bit fields, in TCCRnB. */                        \
  AVR_IO_CONST(uint8_t, CS##n##2, 2);                              \
  AVR_IO_CONST(uint8_t, CS##n##1, 1);                              \
  AVR_IO_CONST(uint8_t, CS##n##0, 0);                              \
  /* Force output compare bit fields, in TCCRnB. */                \
  AVR_IO_CONST(uint8_t, FOC##n##A, 7);                             \
  AVR_IO_CONST(uint8_t, FOC##n##B, 6);                             \
  /* Interrupt Mask Register bit fields, in TIMSKn. */             \
  AVR_IO_CONST(uint8_t, OCIE##n##B, 2);                            \
  AVR_IO_CONST(uint8_t, OCIE##n##A, 1);                            \
  AVR_IO_CONST(uint8_t, TOIE##n, 0);                               \
  /* Interrupt Flag Register bit fields, in TIFRn. */              \
  AVR_IO_CONST(uint8_t, OCF##n##B, 2);                             \
  AVR_IO_CONST(uint8_t, OCF##n##A, 1);                             \
  AVR_IO_CONST(uint8_t, TOV##n, 0)

DEFINE_AVR_8_BIT_TIMER_COUNTER_REGISTERS_AND_FIELDS(0);  // NOLINT

#define DEFINE_AVR_16_BIT_TIMER_COUNTER_REGISTERS_AND_FIELDS(n)                \
  /* T/C Control Registers A, B and C. */                                      \
  AVR_IO_REGISTER_LINKAGE volatile uint8_t TCCR##n##A, TCCR##n##B, TCCR##n##C; \
  /* T/C Output Compare Registers A, B and C. */                               \
  AVR_IO_REGISTER_LINKAGE volatile uint16_t OCR##n##A, OCR##n##B, OCR##n##C;   \
  /* T/C Interrupt Mask and Flag Registers. */                                 \
  AVR_IO_REGISTER_LINKAGE volatile uint8_t TIMSK##n, TIFR##n;                  \
  /* T/C Counter and Input Capture Registers. */                               \
  AVR_IO_REGISTER_LINKAGE volatile uint16_t TCNT##n, ICR##n;                   \
  /* Compare Match Output Mode bit fields, in TCCRnA. */                       \
  AVR_IO_CONST(uint8_t, COM##n##A1, 7);                                        \
  AVR_IO_CONST(uint8_t, COM##n##A0, 6);                                        \
  AVR_IO_CONST(uint8_t, COM##n##B1, 5);                                        \
  AVR_IO_CONST(uint8_t, COM##n##B0, 4);                                        \
  AVR_IO_CONST(uint8_t, COM##n##C1, 3);                                        \
  AVR_IO_CONST(uint8_t, COM##n##C0, 2);                                        \
  /* Waveform Generation bit fields, in TCCRnA and TCCRnB. */                  \
  AVR_IO_CONST(uint8_t, WGM##n##3, 4); /* TCCRnB */                            \
  AVR_IO_CONST(uint8_t, WGM##n##2, 3); /* TCCRnB */                            \
  AVR_IO_CONST(uint8_t, WGM##n##1, 1); /* TCCRnA */                            \
  AVR_IO_CONST(uint8_t, WGM##n##0, 0); /* TCCRnA */                            \
  /* Clock select bit fields, in TCCRnB. */                                    \
  AVR_IO_CONST(uint8_t, CS##n##2, 2);                                          \
  AVR_IO_CONST(uint8_t, CS##n##1, 1);                                          \
  AVR_IO_CONST(uint8_t, CS##n##0, 0);                                          \
  /* Input capture bit fields, in TCCRnB. */                                   \
  AVR_IO_CONST(uint8_t, ICNC##n, 7); /* IC Noise Canceler */                   \
  AVR_IO_CONST(uint8_t, ICES##n, 6); /* IC Edge Select */                      \
  /* Force output compare bit fields, in TCCRnC. */                            \
  AVR_IO_CONST(uint8_t, FOC##n##A, 7);                                         \
  AVR_IO_CONST(uint8_t, FOC##n##B, 6);                                         \
  AVR_IO_CONST(uint8_t, FOC##n##C, 5);                                         \
  /* Interrupt Mask Register bit fields, in TIMSKn. */                         \
  AVR_IO_CONST(uint8_t, ICIE##n, 5);                                           \
  AVR_IO_CONST(uint8_t, OCIE##n##C, 3);                                        \
  AVR_IO_CONST(uint8_t, OCIE##n##B, 2);                                        \
  AVR_IO_CONST(uint8_t, OCIE##n##A, 1);                                        \
  AVR_IO_CONST(uint8_t, TOIE##n, 0);                                           \
  /* Interrupt Flag Register bit fields. */                                    \
  AVR_IO_CONST(uint8_t, ICF##n, 5);                                            \
  AVR_IO_CONST(uint8_t, OCF##n##C, 3);                                         \
  AVR_IO_CONST(uint8_t, OCF##n##B, 2);                                         \
  AVR_IO_CONST(uint8_t, OCF##n##A, 1);                                         \
  AVR_IO_CONST(uint8_t, TOV##n, 0)

DEFINE_AVR_16_BIT_TIMER_COUNTER_REGISTERS_AND_FIELDS(1);  // NOLINT
DEFINE_AVR_16_BIT_TIMER_COUNTER_REGISTERS_AND_FIELDS(3);  // NOLINT
DEFINE_AVR_16_BIT_TIMER_COUNTER_REGISTERS_AND_FIELDS(4);  // NOLINT
DEFINE_AVR_16_BIT_TIMER_COUNTER_REGISTERS_AND_FIELDS(5);  // NOLINT

// Timer/Counter2 has all the registers of an 8-bit T/C, plus some extras.
DEFINE_AVR_8_BIT_TIMER_COUNTER_REGISTERS_AND_FIELDS(2);  // NOLINT
// General Timer/Counter Control Register.
AVR_IO_REGISTER_LINKAGE volatile uint8_t GTCCR;
// Asynchronous Status Register.
AVR_IO_REGISTER_LINKAGE volatile uint8_t ASSR;
/* Asynchronous Status Register bit fields. */
AVR_IO_CONST(uint8_t, EXCLK, 6);
AVR_IO_CONST(uint8_t, AS2, 5);
AVR_IO_CONST(uint8_t, TCN2UB, 4);
AVR_IO_CONST(uint8_t, OCR2AUB, 3);
AVR_IO_CONST(uint8_t, OCR2BUB, 2);
AVR_IO_CONST(uint8_t, TCR2AUB, 1);
AVR_IO_CONST(uint8_t, TCR2BUB, 0);

#define DEFINE_AVR_GPIO_PORT_FIELDS(port, register) \
  AVR_IO_CONST(uint8_t, register##port##7, 7);      \
  AVR_IO_CONST(uint8_t, register##port##6, 6);      \
  AVR_IO_CONST(uint8_t, register##port##5, 5);      \
  AVR_IO_CONST(uint8_t, register##port##4, 4);      \
  AVR_IO_CONST(uint8_t, register##port##3, 3);      \
  AVR_IO_CONST(uint8_t, register##port##2, 2);      \
  AVR_IO_CONST(uint8_t, register##port##1, 1);      \
  AVR_IO_CONST(uint8_t, register##port##0, 0)

#define DEFINE_AVR_GPIO_PORT_REGISTERS_AND_FIELDS(port)                      \
  AVR_IO_REGISTER_LINKAGE volatile uint8_t PIN##port, DDR##port, PORT##port; \
  DEFINE_AVR_GPIO_PORT_FIELDS(port, PIN);                                    \
  DEFINE_AVR_GPIO_PORT_FIELDS(port, DD);                                     \
  DEFINE_AVR_GPIO_PORT_FIELDS(port, P)

DEFINE_AVR_GPIO_PORT_REGISTERS_AND_FIELDS(A);
DEFINE_AVR_GPIO_PORT_REGISTERS_AND_FIELDS(B);
DEFINE_AVR_GPIO_PORT_REGISTERS_AND_FIELDS(C);
DEFINE_AVR_GPIO_PORT_REGISTERS_AND_FIELDS(D);
DEFINE_AVR_GPIO_PORT_REGISTERS_AND_FIELDS(E);
DEFINE_AVR_GPIO_PORT_REGISTERS_AND_FIELDS(F);
DEFINE_AVR_GPIO_PORT_REGISTERS_AND_FIELDS(G);

AVR_IO_REGISTER_LINKAGE volatile uint8_t MCUSR;
constexpr uint8_t JTRF = 4;
constexpr uint8_t WDRF = 3;
constexpr uint8_t BORF = 2;
constexpr uint8_t EXTRF = 1;
constexpr uint8_t PORF = 0;

#define PIN_A0 (54)
#define PIN_A1 (55)
#define PIN_A2 (56)
#define PIN_A3 (57)
#define PIN_A4 (58)
#define PIN_A5 (59)
#define PIN_A6 (60)
#define PIN_A7 (61)
#define PIN_A8 (62)
#define PIN_A9 (63)
#define PIN_A10 (64)
#define PIN_A11 (65)
#define PIN_A12 (66)
#define PIN_A13 (67)
#define PIN_A14 (68)
#define PIN_A15 (69)

#define PIN_WIRE_SDA (20)
#define PIN_WIRE_SCL (21)

#endif  // MCUCORE_EXTRAS_HOST_ARDUINO_AVR_IO_H_
