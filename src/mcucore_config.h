#ifndef MCUCORE_SRC_MCUCORE_CONFIG_H_
#define MCUCORE_SRC_MCUCORE_CONFIG_H_

// Controls the set of features available (enabled/disabled) in McuCore, which
// was originally created as part of Tiny Alpaca Server.
//
// See also mcucore_platform.h, which expresses the set of features provided by
// the platform.
//
// This file should only define macros (and possibly constants), and not include
// (and hence export) anything else. And this file should not depend on any
// symbols (e.g. macros) defined by mcucore_platform.h; this means we can not
// check here whether MCU_EMBEDDED_TARGET or MCU_HOST_TARGET is defined.
//
// Author: james.synge@gmail.com

#ifdef MCU_EMBEDDED_TARGET
#error "This file must be included by mcucore_platform.h before defining macros."
#endif  // MCU_EMBEDDED_TARGET

#ifdef ARDUINO
// After development, for the embedded target, we *should* leave MCU_CHECK
// enabled, but not MCU_VLOG or MCU_DCHECK. Ideally we'd have a build system
// that supports easily producing debug and release variants.

#if !defined(MCU_ENABLE_CHECK) && !defined(MCU_DISABLE_CHECK)
#define MCU_ENABLE_CHECK
#endif  // !MCU_ENABLE_CHECK && !MCU_DISABLE_CHECK

#if !defined(MCU_ENABLE_DCHECK) && !defined(MCU_DISABLE_DCHECK)
#define MCU_ENABLE_DCHECK
#endif  // !MCU_ENABLE_DCHECK && !MCU_DISABLE_DCHECK

#ifndef MCU_ENABLED_VLOG_LEVEL
#define MCU_ENABLED_VLOG_LEVEL 1
#endif  //! MCU_ENABLED_VLOG_LEVEL

#else  // !ARDUINO

#ifndef MCU_ENABLED_VLOG_LEVEL
#define MCU_ENABLED_VLOG_LEVEL 4
#endif  //! MCU_ENABLED_VLOG_LEVEL

#if !defined(MCU_ENABLE_CHECK) && !defined(MCU_DISABLE_CHECK)
#define MCU_ENABLE_CHECK
#endif  // !MCU_ENABLE_CHECK && !MCU_DISABLE_CHECK

#if !defined(MCU_ENABLE_DCHECK) && !defined(MCU_DISABLE_DCHECK)
#ifndef NDEBUG
#define MCU_ENABLE_DCHECK
#endif  // !NDEBUG
#endif  // !MCU_ENABLE_DCHECK && !MCU_DISABLE_DCHECK

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
