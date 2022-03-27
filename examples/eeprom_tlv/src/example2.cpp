#include "example2.h"

MCU_DEFINE_NAMED_DOMAIN(ExampleDomain2, 18);

void Example2Func() {
  auto domain = MCU_DOMAIN(ExampleDomain2);
  MCU_CHECK_EQ(domain.value(), 18);
}
