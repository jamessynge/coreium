#include "mcucore_platform.h"

#pragma clang diagnostic push
#pragma clang diagnostic warning "-W#pragma-messages"

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

#pragma clang diagnostic pop
