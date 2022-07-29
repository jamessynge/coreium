#include "extras/host/arduino/wcharacter.h"

#include "absl/strings/ascii.h"

bool isAlphaNumeric(const char c) { return absl::ascii_isalnum(c); }
bool isGraph(const char c) { return absl::ascii_isgraph(c); }
bool isPrintable(const char c) { return absl::ascii_isprint(c); }
bool isUpperCase(const char c) { return absl::ascii_isupper(c); }
