#ifndef __GC_h
#define __GC_h

#include <new>
#include <cstddef>
#include <iterator>

namespace tcii::ex
{ // begin namespace tcii::ex


//////////////////////////////////////////////////////////
//
// BlockInfo class
// =========
struct BlockInfo
{
  unsigned flag;
  unsigned size;
  BlockInfo* prev;
  BlockInfo* next;

}; // BlockInfo
unsigned constexpr infoSize = sizeof(BlockInfo);
unsigned constexpr pairSize = 2 * infoSize;
/////////////////////////////
//
// Allocator class
// =========
class Allocator
{
public:
  static constexpr auto minBlockSize = 16u;
  static constexpr auto heapSize = 1048576u;

  static void initialize(unsigned = heapSize){
    if (_instance == nullptr)
      _instance = new Allocator();

  };
  template <typename T> static T* allocate(unsigned n = 1);
  static void free(void*);
  static void printMemoryMap();
  static void exit();

private:
  static Allocator* _instance;

  struct {
    BlockInfo header{0, heapSize - sizeof(BlockInfo) * 2, nullptr, nullptr};
    std::byte data[heapSize - sizeof(BlockInfo) * 2]{};
    BlockInfo footer{0, 0, &header};
  } heap;

  BlockInfo* free_blocks = &(heap.header);
  Allocator() = default;

}; // Allocator
template<typename T>
T * Allocator::allocate(unsigned n) {
  auto search = _instance->free_blocks;
  auto start = search;

  auto type_size = n * sizeof(T);
  auto full_size = type_size + 2 * infoSize;


  auto* hf = &(_instance->heap.footer);
  auto heeee = sizeof(_instance->heap);
  auto dif = hf - search;
  auto suposto = search + heapSize;
  auto soos = heapSize;


  if (search == nullptr)
    throw std::bad_alloc();

  do {
    const auto target_b = reinterpret_cast<std::byte *>(search);

    if (search->size == type_size || ((search->size > full_size) && (search->size - full_size <= minBlockSize))) {

      // Tirar bloco da lista circular
      if (search->prev != nullptr)
        search->prev->next = search->next;
      if (search->next != nullptr)
        search->next->prev = search->prev;

      // Marcar bloco como alocado
      BlockInfo* footer = reinterpret_cast<BlockInfo *>(target_b + infoSize + search->size);
      footer->flag = 1;
      search->flag = 1;

      _instance->free_blocks = search->next;

      return reinterpret_cast<T*>(search + 1);
    }
    if (search->size > full_size) {

      // Bloco alocado:
      BlockInfo* oldFooter = reinterpret_cast<BlockInfo *>(target_b + infoSize + search->size);
      oldFooter->flag = 1;

      BlockInfo* newHeader = reinterpret_cast<BlockInfo *>(target_b + infoSize + type_size + infoSize);
      newHeader-> flag = 1;
      newHeader->size = type_size;

      // Bloco livre:
      BlockInfo* newFooter = reinterpret_cast<BlockInfo *>(target_b + infoSize + type_size);
      newFooter->flag = 0;
      newFooter->prev = search;

      search->size -= full_size;

      return reinterpret_cast<T*>(newHeader + 1);
    }

    search = search->next;
  }while (search != start && search != nullptr);

  throw std::bad_alloc();
}


} // end namespace tcii::ex

#endif // __GC_h
