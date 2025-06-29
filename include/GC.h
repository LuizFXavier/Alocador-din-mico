#ifndef __GC_h
#define __GC_h

#include <iostream>
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
    if (_instance == nullptr) {
      _instance = new Allocator();
      _instance->heap.header.next = &(_instance->heap.header);
      _instance->heap.header.prev = &(_instance->heap.header);
    }

  };
  template <typename T> static T* allocate(unsigned n = 1);
  static void free(void* p);
  static void printMemoryMap();
  static void exit(){delete _instance; _instance = nullptr;};

private:
  static Allocator* _instance;

  struct {
    BlockInfo header{0, heapSize - sizeof(BlockInfo) * 2, nullptr, nullptr};
    std::byte data[heapSize - sizeof(BlockInfo) * 2]{};
    BlockInfo footer{0, 0, &header};
  } heap;

  BlockInfo* free_blocks = &(heap.header);
  Allocator() = default;

  static BlockInfo* left_header(const BlockInfo * target_header) {

    if (target_header == &(_instance->heap.header))
      return nullptr;

    auto left_footer = target_header - 1;

    if (left_footer->flag == 1)
      return nullptr;

    return left_footer->prev;
  }
  static BlockInfo* right_free(BlockInfo * target_header) {

    auto target_footer = reinterpret_cast<BlockInfo*>(reinterpret_cast<std::byte*>(target_header) + infoSize + target_header->size);

    if (target_footer == &(_instance->heap.footer))
      return nullptr;

    auto right_header = target_footer + 1;

    if (right_header->flag == 1)
      return nullptr;

    return right_header;
  };

}; // Allocator
template<typename T>
T * Allocator::allocate(unsigned n) {
  auto search = _instance->free_blocks;
  auto start = search;

  auto type_size = n * sizeof(T);
  auto full_size = type_size + 2 * infoSize;

  if (search == nullptr)
    throw std::bad_alloc();

  do {
    const auto target_b = reinterpret_cast<std::byte *>(search);

    // Alocar bloco inteiro
    if (search->size == type_size || ((search->size > type_size) && ((int)search->size - (int)full_size <= (int)minBlockSize))) {

      // Tirar bloco da lista circular
      if (search->prev != nullptr)
        search->prev->next = search->next;
      if (search->next != nullptr)
        search->next->prev = search->prev;

      if (search->next != search)
        _instance->free_blocks = search->next;
      else
        _instance->free_blocks = nullptr;

      // Marcar bloco como alocado
      BlockInfo* footer = reinterpret_cast<BlockInfo *>(target_b + infoSize + search->size);
      footer->flag = 1;
      search->flag = 1;

      return reinterpret_cast<T*>(search + 1);
    }
    // Fragmentar bloco
    if (search->size > full_size) {

      // Bloco alocado:
      BlockInfo* oldFooter = reinterpret_cast<BlockInfo *>(target_b + infoSize + search->size);
      oldFooter->flag = 1;

      BlockInfo* newHeader = reinterpret_cast<BlockInfo *>(target_b + infoSize + search->size - type_size - infoSize);
      newHeader-> flag = 1;
      newHeader->size = type_size;

      // Bloco livre:
      BlockInfo* newFooter = newHeader - 1;
      newFooter->flag = 0;
      newFooter->prev = search;

      search->size -= full_size;
      T* t = reinterpret_cast<T*>(newHeader + 1);
      return t;
    }

    search = search->next;
  }while (search != start && search != nullptr);

  throw std::bad_alloc();
}

void inline
Allocator::free(void * p) {

  auto p_b = static_cast<std::byte *>(p);
  auto target_block = reinterpret_cast<BlockInfo*>(p_b - infoSize);
  auto target_bytes = reinterpret_cast<std::byte *>(target_block);
  auto left_block = left_header(target_block);
  auto right_block = right_free(target_block);

  // Marcar bloco como desalocado
  target_block->flag = 0;
  target_block->next = target_block;
  target_block->prev = target_block;
  auto target_footer = reinterpret_cast<BlockInfo *>(target_bytes + infoSize + target_block->size);
  target_footer->flag = 0;
  target_footer->prev = target_block;

  // Unir este bloco com o da esquerda
  if (left_block) {
    target_footer->prev = left_block;

    left_block->size = left_block->size + 2 * infoSize + target_block->size;

    target_block = left_block;
    target_bytes = reinterpret_cast<std::byte *>(left_block);
  }
  // Unir o bloco da direita com este
  if (right_block) {
    if (!left_block) {

      target_block->prev = right_block->prev;
      target_block->prev->next = target_block;
    }
    target_block->next = right_block->next;
    target_block->next->prev = target_block;
    target_block->size = target_block->size + 2 * infoSize + right_block->size;

    auto right_footer = reinterpret_cast<BlockInfo *>(target_bytes + infoSize + target_block->size);
    right_footer->prev = target_block;
  }

  if (_instance->free_blocks == nullptr) {
    _instance->free_blocks = target_block;
    return;
  }

  if (!left_block && !right_block) {
    auto search = _instance->free_blocks;
    auto start = search;

    while (true) {
      auto entre_dois = search < target_block && target_block < search->next;
      auto final_circulo = (search > search->next) && (search < target_block || target_block < search->next);
      auto unitaria = search == search->next;

      if (entre_dois || final_circulo || unitaria) {
        target_block->next = search->next;
        search->next->prev = target_block;
        target_block->prev = search;
        search->next = target_block;
        return;
      }
      search = search->next;
    }
  }
}

void inline
Allocator::printMemoryMap() {
  unsigned free_space = 0;
  auto block = &_instance->heap.header;
  auto byte_block = reinterpret_cast<std::byte *>(block);

  while (true) {
    std::cout << "Address: " << block << std::endl;
    std::cout << "Flag: " << block->flag << std::endl;
    std::cout << "Size: " << block->size << std::endl << std::endl;

    if (block->flag == 0)
      free_space += block->size;

    auto next_address = reinterpret_cast<BlockInfo *>(byte_block + infoSize + block->size + infoSize);
    if (next_address < &_instance->heap.footer) {
    // std::cout << "Next:   " << next_address << std::endl;
      block = next_address;
      byte_block = reinterpret_cast<std::byte *>(block);
    }
    else
      break;

    // if (a > 5)
    //   break;
  }

  std::cout << "Espaço disponível: " << free_space << std::endl;

}
} // end namespace tcii::ex

#endif // __GC_h
