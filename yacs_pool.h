#ifndef YACS_POOL_H
#define YACS_POOL_H

#include <cstdint>
#include <algorithm>
#include <array>

#include "yacs_component.h"

namespace yacs {
  constexpr size_t align_to(const size_t &size, const size_t aligment) {
    return (size + aligment - 1) / aligment * aligment;
  }

  class typeless_pool {
  public:
    typeless_pool(const size_t size, const size_t piece_size, const size_t piece_aligment) noexcept;
    ~typeless_pool() noexcept;

    typeless_pool(const typeless_pool &copy) noexcept = delete;
    typeless_pool & operator=(const typeless_pool &copy) noexcept = delete;
    typeless_pool(typeless_pool &&move) noexcept;
    typeless_pool & operator=(typeless_pool &&move) noexcept;

    void* allocate();
    void free(void* block) noexcept;
    void clear() noexcept;

    template <typename T, typename... Args>
    T* create(Args&&... args) {
      YACS_ASSERT(sizeof(T) <= m_piece_size && "Type size is more than this pool piece size");
      YACS_ASSERT(alignof(T) <= m_piece_aligment && "Type aligment is more than this pool piece aligment");
      auto ptr = allocate();
      return new (ptr) T(std::forward<Args>(args)...);
    }

    template <typename T>
    void destroy(T* obj) noexcept {
      YACS_ASSERT(sizeof(T) <= m_piece_size && "Type size is more than this pool piece size");
      YACS_ASSERT(alignof(T) <= m_piece_aligment && "Type aligment is more than this pool piece aligment");
      if (obj == nullptr) return;
      obj->~T();
      free(obj);
    }

    size_t size() const noexcept;
    size_t block_size() const noexcept;
    size_t piece_size() const noexcept;
    size_t piece_aligment() const noexcept;
  private:
    void allocate_block();

    size_t m_size;
    size_t m_piece_size;
    size_t m_piece_aligment;
    size_t m_current;
    char* m_memory;
    void* m_free_mem;
  };
}

// template <typename T, size_t N>
// struct static_container {
//   std::array<T, N> array;
//   static_container* next;
//
//   static_container() : next(nullptr) {}
//   static_container(const T &default_value) : next(nullptr) {
//     for (size_t i = 0; i < array.size(); ++i) {
//       array[i] = default_value;
//     }
//   }
// };
//
// template <typename T, size_t N>
// static_container<T, N>* advance_static_container(static_container<T, N>* cont, const size_t &index) {
//   static_container<T, N>* current = cont;
//   for (size_t i = 0; i < index && current != nullptr; ++i) {
//     current = current->next;
//   }
//
//   return current;
// }
//
// template <typename T, size_t N>
// const static_container<T, N>* advance_static_container(const static_container<T, N>* cont, const size_t &index) {
//   auto current = cont;
//   for (size_t i = 0; i < index && current != nullptr; ++i) {
//     current = current->next;
//   }
//
//   return current;
// }

#endif //YACS_POOL_H
