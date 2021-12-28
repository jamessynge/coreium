#include "mcucore_config.h"

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "mcucore_platform.h"

namespace mcucore {
namespace test {
namespace {

TEST(ConfigTest, LogInfo) {
#define LOG_MACRO(m) LOG(INFO) << #m << ": " << m;

  LOG_MACRO(TAS_EMBEDDED_TARGET);
  LOG_MACRO(TAS_HOST_TARGET);
  LOG_MACRO(TAS_ENABLE_DEBUGGING);

#ifdef TAS_ENABLED_VLOG_LEVEL
  LOG_MACRO(TAS_ENABLED_VLOG_LEVEL);
#endif  // TAS_ENABLED_VLOG_LEVEL

#ifdef TAS_ENABLE_CHECK
  LOG(INFO) << "TAS_ENABLE_CHECK is defined";
#else   // !TAS_ENABLE_CHECK
  LOG(INFO) << "TAS_ENABLE_CHECK is NOT defined";
#endif  // TAS_ENABLE_CHECK

#ifdef TAS_ENABLE_DCHECK
  LOG(INFO) << "TAS_ENABLE_DCHECK is defined";
#else   // !TAS_ENABLE_DCHECK
  LOG(INFO) << "TAS_ENABLE_DCHECK is NOT defined";
#endif  // TAS_ENABLE_DCHECK
}

}  // namespace
}  // namespace test
}  // namespace mcucore
