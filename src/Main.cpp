#include <iostream>
#include "GC.h"
inline void
allocatorTest() {

  using namespace tcii::ex;

  Allocator::initialize();

  auto p = Allocator::allocate<int>(200);

  struct str{
    double d;
    bool b;
  }* s = Allocator::allocate<str>(1);

  p[0] = 2, p[1] = 3;

  s->b = true;
  s->d = 0.5;

  std::cout << p[0] << ", " << p[1] << std::endl;
  std::cout << s->b << ", " << s->d << std::endl;
}

//
// Main function
//
int
main()
{
  allocatorTest();
  return 0;
}
