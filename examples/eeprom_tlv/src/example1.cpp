#include "example1.h"

MCU_DEFINE_DOMAIN(17);

void Example1Func() {
  auto domain = MCU_DOMAIN(17);
  domain.value();
  MCU_CHECK_EQ(domain.value(), 17);
}
