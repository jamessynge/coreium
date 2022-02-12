#ifndef MCUCORE_SRC_ANY_PRINTABLE_H_
#define MCUCORE_SRC_ANY_PRINTABLE_H_

// AnyPrintable allows anything that we know can be passed to an overload of
// Print::print to be captured and passed around. This is in support of
// preparing responses to Alpaca requests by "concatenating" multiple printable
// values into a single Printable instance via PrintableCat.
//
// For those captured items that reference values to be printed but not
// contained by the captured item, the referenced value needs to outlive the
// AnyPrintable.
//
// Author: james.synge@gmail.com

#include "mcucore_platform.h"
#include "progmem_string.h"
#include "progmem_string_view.h"
#include "string_view.h"
#include "type_traits.h"

namespace mcucore {

class AnyPrintable : public Printable {
  enum EFragmentType {
    kEmpty,
    kProgmemStringView,
    kStringView,
    kPrintable,
    kFlashStringHelper,
    kChar,
    kInteger,
    kUnsignedInteger,
    kFloat,
    kDouble,
  };

 public:
  AnyPrintable();
  // For values that are clearly strings, we allow implicit conversion to
  // AnyPrintable.
  AnyPrintable(StringView value);                  // NOLINT
  AnyPrintable(ProgmemString value);               // NOLINT
  AnyPrintable(ProgmemStringView value);           // NOLINT
  AnyPrintable(const __FlashStringHelper* value);  // NOLINT
  // To avoid implicit conversions of values that aren't (weren't) necessarily
  // strings, we require the conversion to be explicit.
  explicit AnyPrintable(Printable& value);
  explicit AnyPrintable(const Printable& value);
  explicit AnyPrintable(char value);
  explicit AnyPrintable(int16_t value);
  explicit AnyPrintable(uint16_t value);
  explicit AnyPrintable(int32_t value);
  explicit AnyPrintable(uint32_t value);
  explicit AnyPrintable(float value);
  explicit AnyPrintable(double value);

  // If the value is an enum, convert it to an integral type.
  template <typename T, enable_if_t<is_enum<T>::value, bool> = true>
  explicit AnyPrintable(T value)
      : AnyPrintable(static_cast<underlying_type_t<T>>(value)) {}

  // We can't copy a Printable instance, so we prevent temporaries from being
  // passed in.
  AnyPrintable(Printable&& value) = delete;
  AnyPrintable(const Printable&& value) = delete;

  // Copy and assignment operators.
  AnyPrintable(const AnyPrintable&);
  AnyPrintable& operator=(const AnyPrintable&);

  size_t printTo(Print& out) const override;

 private:
  EFragmentType type_;
  union {
    ProgmemStringView psv_;
    StringView view_;
    const __FlashStringHelper* flash_string_helper_;
    char char_;
    int32_t signed_;
    uint32_t unsigned_;
    float float_;
    double double_;
    const Printable* printable_;
  };
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_ANY_PRINTABLE_H_
