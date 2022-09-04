#ifndef MCUCORE_SRC_MCUCORE_H_
#define MCUCORE_SRC_MCUCORE_H_

// This file acts to export all of the headers that would be needed by a program
// (i.e. an Arduino Sketch file) using this library.
//
// Author: james.synge@gmail.com

#include "container/array.h"                  // IWYU pragma: export
#include "container/array_view.h"             // IWYU pragma: export
#include "container/flash_string_table.h"     // IWYU pragma: export
#include "container/serial_map.h"             // IWYU pragma: export
#include "eeprom/eeprom_io.h"                 // IWYU pragma: export
#include "eeprom/eeprom_region.h"             // IWYU pragma: export
#include "eeprom/eeprom_tag.h"                // IWYU pragma: export
#include "eeprom/eeprom_tlv.h"                // IWYU pragma: export
#include "hash/crc32.h"                       // IWYU pragma: export
#include "hash/fnv1a.h"                       // IWYU pragma: export
#include "http1/request_decoder.h"            // IWYU pragma: export
#include "http1/request_decoder_constants.h"  // IWYU pragma: export
#include "json/json_encoder.h"                // IWYU pragma: export
#include "json/json_encoder_helpers.h"        // IWYU pragma: export
#include "log/log.h"                          // IWYU pragma: export
#include "log/log_sink.h"                     // IWYU pragma: export
#include "mcucore_config.h"                   // IWYU pragma: export
#include "mcucore_platform.h"                 // IWYU pragma: export
#include "misc/progmem_ptr.h"                 // IWYU pragma: export
#include "misc/to_unsigned.h"                 // IWYU pragma: export
#include "misc/uuid.h"                        // IWYU pragma: export
#include "platform/avr/jitter_random.h"       // IWYU pragma: export
#include "platform/avr/timer_counter.h"       // IWYU pragma: export
#include "platform/avr/watchdog.h"            // IWYU pragma: export
#include "print/any_printable.h"              // IWYU pragma: export
#include "print/counting_print.h"             // IWYU pragma: export
#include "print/has_insert_into.h"            // IWYU pragma: export
#include "print/has_print_to.h"               // IWYU pragma: export
#include "print/hex_dump.h"                   // IWYU pragma: export
#include "print/hex_escape.h"                 // IWYU pragma: export
#include "print/o_print_stream.h"             // IWYU pragma: export
#include "print/print_misc.h"                 // IWYU pragma: export
#include "print/print_to_buffer.h"            // IWYU pragma: export
#include "print/printable_cat.h"              // IWYU pragma: export
#include "print/stream_to_print.h"            // IWYU pragma: export
#include "semistd/limits.h"                   // IWYU pragma: export
#include "semistd/type_traits.h"              // IWYU pragma: export
#include "semistd/utility.h"                  // IWYU pragma: export
#include "status/status.h"                    // IWYU pragma: export
#include "status/status_code.h"               // IWYU pragma: export
#include "status/status_or.h"                 // IWYU pragma: export
#include "strings/has_progmem_char_array.h"   // IWYU pragma: export
#include "strings/progmem_string.h"           // IWYU pragma: export
#include "strings/progmem_string_data.h"      // IWYU pragma: export
#include "strings/progmem_string_view.h"      // IWYU pragma: export
#include "strings/string_compare.h"           // IWYU pragma: export
#include "strings/string_view.h"              // IWYU pragma: export
#include "strings/tiny_string.h"              // IWYU pragma: export

#endif  // MCUCORE_SRC_MCUCORE_H_
