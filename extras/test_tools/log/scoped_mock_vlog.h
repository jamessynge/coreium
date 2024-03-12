#ifndef MCUCORE_EXTRAS_TEST_TOOLS_LOG_SCOPEDMOCKVLOG_H_
#define MCUCORE_EXTRAS_TEST_TOOLS_LOG_SCOPEDMOCKVLOG_H_

// See absl::ScopedMockLog for the inspiration for this class. For example:
// https://fuchsia.googlesource.com/third_party/abseil-cpp/+/refs/tags/20230125.3/absl/log/scoped_mock_log.h

namespace mcucore {
namespace test {

class ScopedMockVLog final {
 public:
  ScopedMockVLog();

  // ScopedMockLog::StartCapturingLogs()
  //
  // Starts log capturing if the object isn't already doing so. Otherwise
  // crashes.
  //
  // Usually this method is called in the same thread that created this
  // ScopedMockLog. It is the user's responsibility to not call this method if
  // another thread may be calling it or StopCapturingLogs() at the same time.
  // It is undefined behavior to add expectations while capturing logs is
  // enabled.
  void StartCapturingLogs();

  // Implements the mock method:
  //
  //   void Log(LogSeverity severity, absl::string_view file_path,
  //            absl::string_view message);
  //
  // The second argument to Log() is the full path of the source file in
  // which the LOG() was issued.
  //
  // This is a shorthand form, which should be used by most users. Use the
  // `Send` mock only if you want to add expectations for other log message
  // attributes.
  MOCK_METHOD(void, Log,
              (const std::string& file_path, const std::string& message));

 private:
  bool is_capturing_logs_ = false;
};

}  // namespace test
}  // namespace mcucore

#endif  // MCUCORE_EXTRAS_TEST_TOOLS_LOG_SCOPEDMOCKVLOG_H_
