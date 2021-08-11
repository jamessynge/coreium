#ifndef MCUCORE_SRC_STREAM_TO_PRINT_H_
#define MCUCORE_SRC_STREAM_TO_PRINT_H_

// Enables streaming output to Arduino Print instances, including Serial, thus
// simplifying writing code that produces output from multiple items.
//
// This is in a separate header file from o_print_stream.h so that it doesn't
// interfere with the use of any other operator<< whose left-hand-side is a
// Print instance.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "o_print_stream.h"

template <typename T>
::mcucore::OPrintStream operator<<(Print& out, const T& value) {
  ::mcucore::OPrintStream strm(out);
  strm << value;
  return strm;
}

#endif  // MCUCORE_SRC_STREAM_TO_PRINT_H_
