#ifndef MCUCORE_SRC_MCUCORE_CONFIG_H_
#define MCUCORE_SRC_MCUCORE_CONFIG_H_

// Controls the set of features available (enabled/disabled) in McuCore, which
// was originally created as part of Tiny Alpaca Server.
//
// See also mcucore_platform.h, which expresses the set of features provided by
// the platform.
//
// This file should only define macros (and possibly constants), and not include
// (and hence export) anything else.
//
// Author: james.synge@gmail.com

#ifdef ARDUINO
// After development, for the embedded target, we *should* leave MCU_CHECK
// enabled, but not MCU_VLOG or MCU_DCHECK.

#ifndef MCU_ENABLED_VLOG_LEVEL
#define MCU_ENABLED_VLOG_LEVEL 3
#endif  //! MCU_ENABLED_VLOG_LEVEL

#ifndef MCU_ENABLE_CHECK
#define MCU_ENABLE_CHECK
#endif  // !MCU_ENABLE_CHECK

#ifdef MCU_ENABLE_DCHECK
#undef MCU_ENABLE_DCHECK
#endif  // MCU_ENABLE_DCHECK

// #ifndef MCU_ENABLE_DCHECK
// #define MCU_ENABLE_DCHECK
// #endif  // !MCU_ENABLE_DCHECK

#else  // !ARDUINO

#ifndef MCU_ENABLED_VLOG_LEVEL
#define MCU_ENABLED_VLOG_LEVEL 4
#endif  //! MCU_ENABLED_VLOG_LEVEL

#ifndef MCU_ENABLE_CHECK
#define MCU_ENABLE_CHECK
#endif  // !MCU_ENABLE_CHECK

#ifndef MCU_ENABLE_DCHECK
#ifndef NDEBUG
#define MCU_ENABLE_DCHECK
#endif  // !NDEBUG
#endif  // !MCU_ENABLE_DCHECK

#endif  // ARDUINO

// These control whether or not MCU_VLOG, MCU_CHECK and MCU_DCHECK include the
// location of their invocation when they log a message.

#if !defined(MCU_ENABLE_VLOG_LOCATION) && !defined(MCU_DISABLE_VLOG_LOCATION)
#define MCU_ENABLE_VLOG_LOCATION
#endif

#if !defined(MCU_ENABLE_CHECK_LOCATION) && !defined(MCU_DISABLE_CHECK_LOCATION)
#define MCU_ENABLE_CHECK_LOCATION
#endif

#if !defined(MCU_ENABLE_DCHECK_LOCATION) && \
    !defined(MCU_DISABLE_DCHECK_LOCATION)
#define MCU_ENABLE_DCHECK_LOCATION
#endif

#endif  // MCUCORE_SRC_MCUCORE_CONFIG_H_
