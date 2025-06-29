#include <iostream>
#include "GC.h"
inline void
allocatorTest() {

  using namespace tcii::ex;

  Allocator::initialize();

  auto p1 = Allocator::allocate<int>(2);
  auto p2 = Allocator::allocate<int>(1);
  auto p3 = Allocator::allocate<int>(1);
  auto p4 = Allocator::allocate<int>(5);

  Allocator::printMemoryMap();
  Allocator::free(p2);
  std::cout << "----------p2---------------" << std::endl;
  Allocator::printMemoryMap();
  Allocator::free(p3);

  std::cout << "----------p3--------------" << std::endl;
  Allocator::printMemoryMap();

  Allocator::free(p1);

  std::cout << "----------p1--------------" << std::endl;
  Allocator::printMemoryMap();
  Allocator::exit();
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
