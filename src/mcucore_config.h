#ifndef MCUCORE_SRC_MCUCORE_CONFIG_H_
#define MCUCORE_SRC_MCUCORE_CONFIG_H_

// Controls the set of features available (enabled/disabled) in the utils of
// Tiny Alpaca Server. (This is in contrast to platform.h, which expresses the
// set of features provided by the platform.)
//
// This file should only define macros (and possibly constants), and not include
// (and hence export) anything else.
//
// Author: james.synge@gmail.com

#ifdef ARDUINO
// After development, for the embedded target, we *should* leave TAS_CHECK
// enabled, but not TAS_VLOG or TAS_DCHECK.

#ifndef TAS_ENABLED_VLOG_LEVEL
#define TAS_ENABLED_VLOG_LEVEL 3
#endif  //! TAS_ENABLED_VLOG_LEVEL

#ifndef TAS_ENABLE_CHECK
#define TAS_ENABLE_CHECK
#endif  // !TAS_ENABLE_CHECK

#ifdef TAS_ENABLE_DCHECK
#undef TAS_ENABLE_DCHECK
#endif  // TAS_ENABLE_DCHECK

// #ifndef TAS_ENABLE_DCHECK
// #define TAS_ENABLE_DCHECK
// #endif  // !TAS_ENABLE_DCHECK

#else  // !ARDUINO

#ifndef TAS_ENABLED_VLOG_LEVEL
#define TAS_ENABLED_VLOG_LEVEL 4
#endif  //! TAS_ENABLED_VLOG_LEVEL

#ifndef TAS_ENABLE_CHECK
#define TAS_ENABLE_CHECK
#endif  // !TAS_ENABLE_CHECK

#ifndef TAS_ENABLE_DCHECK
#ifndef NDEBUG
#define TAS_ENABLE_DCHECK
#endif  // !NDEBUG
#endif  // !TAS_ENABLE_DCHECK

#endif  // ARDUINO

#endif  // MCUCORE_SRC_MCUCORE_CONFIG_H_
