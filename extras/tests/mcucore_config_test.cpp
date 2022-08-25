#include "mcucore_config.h"

#include "absl/log/log.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"

namespace mcucore {
namespace test {
namespace {

TEST(ConfigTest, LogInfo) {
#define LOG_MACRO(m) LOG(INFO) << #m << ": " << m;

  LOG_MACRO(MCU_EMBEDDED_TARGET);
  LOG_MACRO(MCU_HOST_TARGET);
  LOG_MACRO(MCU_ENABLE_DEBUGGING);

#ifdef MCU_ENABLED_VLOG_LEVEL
  LOG_MACRO(MCU_ENABLED_VLOG_LEVEL);
#endif  // MCU_ENABLED_VLOG_LEVEL

#ifdef MCU_ENABLE_CHECK
  LOG(INFO) << "MCU_ENABLE_CHECK is defined";
#else   // !MCU_ENABLE_CHECK
  LOG(INFO) << "MCU_ENABLE_CHECK is NOT defined";
#endif  // MCU_ENABLE_CHECK

#ifdef MCU_ENABLE_DCHECK
  LOG(INFO) << "MCU_ENABLE_DCHECK is defined";
#else   // !MCU_ENABLE_DCHECK
  LOG(INFO) << "MCU_ENABLE_DCHECK is NOT defined";
#endif  // MCU_ENABLE_DCHECK
}

}  // namespace
}  // namespace test
}  // namespace mcucore
