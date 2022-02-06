#ifndef MCUCORE_SRC_JSON_ENCODER_H_
#define MCUCORE_SRC_JSON_ENCODER_H_

// Supports writing a JSON Structure (i.e. an Object or an Array), with values
// that are numbers, bools, strings or JSON structures. To avoid allocation of
// memory for storing a serialized JSON structure, we don't add properties and
// elements before serializing. Instead we depend on calling JsonPropertySource
// and JsonElementSource instances that are created by the caller, which puts
// the caller in control of where the items in the structures come from.
//
// Usage examples can be found in the test file.
//
// Author: james.synge@gmail.com

#include "any_printable.h"
#include "mcucore_platform.h"

namespace mcucore {

class JsonArrayEncoder;
class JsonObjectEncoder;

// API of classes that supply the elements of a JSON Array.
class JsonElementSource {
 public:
  virtual ~JsonElementSource();

  // Provides the elements of the JSON Array by calling the appropriate
  // AddXyzElement method of `encoder`.
  virtual void AddTo(JsonArrayEncoder& encoder) const = 0;
};

// API of classes that supply the elements of a JSON Object.
class JsonPropertySource {
 public:
  virtual ~JsonPropertySource();

  // Provides the properties of the JSON Object by calling the appropriate
  // AddXyzProperty method of `encoder`.
  virtual void AddTo(JsonObjectEncoder& encoder) const = 0;
};

// IDEA: Add a JsonEncoderOptions struct, similar to the kwargs for Python's
// json.dump function (i.e. to support the amount and kind of whitespace between
// items, including support for indenting or omitting all whitespace).

// Base class for the object and array encoders.
class AbstractJsonEncoder {
 protected:
  explicit AbstractJsonEncoder(Print& out);

  // Prints the comma between elements in an array and between properties in an
  // object.
  void StartItem();

  void EncodeChildArray(const JsonElementSource& source);
  void EncodeChildObject(const JsonPropertySource& source);

  void PrintString(const Printable& printable);

  Print& out_;

 private:
  // Make AbstractJsonEncoder non-copyable; also makes it non-moveable.
  AbstractJsonEncoder(const AbstractJsonEncoder&) = delete;
  AbstractJsonEncoder& operator=(const AbstractJsonEncoder&) = delete;

  // True if we've not yet encoded the first item in the array or object; false
  // after that.
  bool first_;
};

// JSON encoder for arrays.
class JsonArrayEncoder : public AbstractJsonEncoder {
 public:
  void AddIntElement(const int32_t value);
  void AddUIntElement(const uint32_t value);
  void AddFloatElement(float value);
  void AddDoubleElement(double value);
  void AddBooleanElement(const bool value);
  void AddStringElement(const AnyPrintable& value);
  void AddStringElement(const Printable& value);
  void AddArrayElement(const JsonElementSource& source);
  void AddObjectElement(const JsonPropertySource& source);

  static void Encode(const JsonElementSource& source, Print& out);
  static size_t EncodedSize(const JsonElementSource& source);

 private:
  friend class AbstractJsonEncoder;

  // Called from Encode or EncodeSize.
  explicit JsonArrayEncoder(Print& out);

  ~JsonArrayEncoder();
};

// JSON encoder for objects.
class JsonObjectEncoder : public AbstractJsonEncoder {
 public:
  void AddIntProperty(const AnyPrintable& name, int32_t value);
  void AddUIntProperty(const AnyPrintable& name, uint32_t value);
  void AddFloatProperty(const AnyPrintable& name, float value);
  void AddDoubleProperty(const AnyPrintable& name, double value);
  void AddBooleanProperty(const AnyPrintable& name, const bool value);
  void AddStringProperty(const AnyPrintable& name, const AnyPrintable& value);
  void AddStringProperty(const AnyPrintable& name, const Printable& value);
  void AddArrayProperty(const AnyPrintable& name,
                        const JsonElementSource& source);
  void AddObjectProperty(const AnyPrintable& name,
                         const JsonPropertySource& source);

  static void Encode(const JsonPropertySource& source, Print& out);
  static size_t EncodedSize(const JsonPropertySource& source);

 private:
  friend class AbstractJsonEncoder;

  // Called from Encode or EncodeSize.
  explicit JsonObjectEncoder(Print& out);

  ~JsonObjectEncoder();

  void StartProperty(const AnyPrintable& name);
};

}  // namespace mcucore

#endif  // MCUCORE_SRC_JSON_ENCODER_H_
