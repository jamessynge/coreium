#include "mcucore_platform.h"

#if MCU_CONFIG_COMPILE_TIME_MESSAGES

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic warning "-W#pragma-messages"
#endif  // __clang__

#ifdef MCU_EMBEDDED_TARGET
#if MCU_EMBEDDED_TARGET
#pragma message("MCU_EMBEDDED_TARGET is non-zero")
#else
#pragma message("MCU_EMBEDDED_TARGET is zero")
#endif
#else
#pragma message("MCU_EMBEDDED_TARGET is undefined")
#endif

#ifdef MCU_HOST_TARGET
#if MCU_HOST_TARGET
#pragma message("MCU_HOST_TARGET is non-zero")
#else
#pragma message("MCU_HOST_TARGET is zero")
#endif
#else
#pragma message("MCU_HOST_TARGET is undefined")
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif  // __clang__

#endif  // MCU_CONFIG_COMPILE_TIME_MESSAGES

namespace mcucore {

// See extras/futures/time.h for another approach to representing time.
MillisT ElapsedMillis(MillisT start_time) { return millis() - start_time; }

}  // namespace mcucore
