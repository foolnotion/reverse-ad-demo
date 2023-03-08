#include "reverse-ad-demo/reverse-ad-demo.hpp"

auto main() -> int
{
  auto const result = name();

  return result == "reverse-ad-demo" ? 0 : 1;
}
