#include "string_compare.h"

namespace mcucore {

bool operator==(const ProgmemStringView& a, const StringView& b) {
  return a.Equal(b.data(), b.size());
}
bool operator==(const StringView& a, const ProgmemStringView& b) {
  return b == a;
}
bool ExactlyEqual(const ProgmemStringView& a, const StringView& b) {
  return a == b;
}
bool operator!=(const ProgmemStringView& a, const StringView& b) {
  return !(a == b);
}
bool operator!=(const StringView& a, const ProgmemStringView& b) {
  return !(b == a);
}
bool CaseEqual(const ProgmemStringView& a, const StringView& b) {
  return a.CaseEqual(b.data(), b.size());
}
bool CaseEqual(const StringView& a, const ProgmemStringView& b) {
  return CaseEqual(b, a);
}
bool LoweredEqual(const ProgmemStringView& a, const StringView& b) {
  return a.LoweredEqual(b.data(), b.size());
}
bool StartsWith(const StringView& text, const ProgmemStringView& prefix) {
  return prefix.IsPrefixOf(text.data(), text.size());
}
bool StartsWith(const ProgmemStringView& text, const StringView& prefix) {
  // text will typically be bigger than prefix, at least when called by
  // RequestDecoder.
  if (text.size() > prefix.size()) {
    return prefix.empty() || text.substr(0, prefix.size()) == prefix;
  } else if (text.size() == prefix.size()) {
    return text == prefix;
  } else {
    return false;
  }
}
bool SkipPrefix(StringView& text, const ProgmemStringView& prefix) {
  if (StartsWith(text, prefix)) {
    text.remove_prefix(prefix.size());
    return true;
  }
  return false;
}

}  // namespace mcucore
