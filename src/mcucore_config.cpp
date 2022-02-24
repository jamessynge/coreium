#include "mcucore_config.h"

#pragma clang diagnostic push
#pragma clang diagnostic warning "-W#pragma-messages"

#if __cplusplus == 201103L
#pragma message("C++ 2011")
#elif __cplusplus < 201103L
#error("C++ before 2011")
#elif __cplusplus == 201402L
#pragma message("C++ 2014")
#elif __cplusplus < 201402L
#pragma message("C++ between 2011 and 2014")
#elif __cplusplus == 201703L
#pragma message("C++ 2017")
#elif __cplusplus < 201703L
#pragma message("C++ between 2014 and 2017")
#elif __cplusplus == 202002L
#pragma message("C++ 2020")
#elif __cplusplus < 202002L
#pragma message("C++ between 2017 and 2020")
#else
#pragma message("C++ beyond 2020")
#endif

#pragma clang diagnostic pop
