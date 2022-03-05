#include "flash_string_table.h"

#include "logging.h"

namespace mcucore {

namespace flash_string_table_internal {

const __FlashStringHelper* FlashStringTableElement::ToFlashStringHelper()
    const {
  // `this` is an address in flash.
  MCU_DCHECK_EQ(static_cast<const void*>(this),
                static_cast<const void*>(&ptr_));
  const void* addr_in_flash = &ptr_;
  const void* flash_string_ptr = pgm_read_ptr_near(addr_in_flash);
  return static_cast<const __FlashStringHelper*>(flash_string_ptr);
}

}  // namespace flash_string_table_internal

}  // namespace mcucore
