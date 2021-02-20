#ifndef YACS_POOL_H
#define YACS_POOL_H

#include <cstdint>
#include <algorithm>
#include <array>
#include <cassert>

#include "yacs_component.h"

namespace yacs {
  constexpr size_t align_to(const size_t &size, const size_t aligment) {
    return (size + aligment - 1) / aligment * aligment;
  }
  
  template <typename T, size_t N>
  struct static_container {
    std::array<T, N> array;
    static_container* next;

    static_container() : next(nullptr) {}
    static_container(const T &default_value) : next(nullptr) {
      for (size_t i = 0; i < array.size(); ++i) {
        array[i] = default_value;
      }
    }
  };

  template <typename T, size_t N>
  static_container<T, N>* advance_static_container(static_container<T, N>* cont, const size_t &index) {
    static_container<T, N>* current = cont;
    for (size_t i = 0; i < index && current != nullptr; ++i) {
      current = current->next;
    }

    return current;
  }
  
  template <typename T, size_t N>
  const static_container<T, N>* advance_static_container(const static_container<T, N>* cont, const size_t &index) {
    auto current = cont;
    for (size_t i = 0; i < index && current != nullptr; ++i) {
      current = current->next;
    }

    return current;
  }

  class typeless_pool {
  public:
    typeless_pool(const size_t &type_size, const size_t &type_aligment, const size_t &block_size) : 
      type_size(type_size), 
      type_aligment(type_aligment), 
      block_size(block_size), 
      current_size(0), 
      memory(nullptr), 
      free_slots(nullptr) 
    {
      ASSERT(type_size >= sizeof(char*));
      allocate_block();
    }

    typeless_pool(typeless_pool &&pool) : 
      type_size(pool.type_size), 
      type_aligment(pool.type_aligment), 
      block_size(pool.block_size), 
      current_size(pool.current_size), 
      memory(pool.memory), 
      free_slots(pool.free_slots) 
    {
      pool.memory = nullptr;
      pool.current_size = 0;
      pool.free_slots = nullptr;
    }

    ~typeless_pool() {
      clear();
    }

    template<typename T, typename... Args>
    T* create(Args&&... args) {
      ASSERT(sizeof(T) == type_size);
      ASSERT(alignof(T) == type_aligment);

      auto ptr = allocate();
      auto valid_ptr = new (ptr) T(std::forward<Args>(args)...);
      return valid_ptr;
    }

    template<typename T>
    void destroy(T* ptr) {
      if (ptr == nullptr) return;
      ASSERT(sizeof(T) == type_size);
      ASSERT(alignof(T) == type_aligment);

      ptr->~T();

      reinterpret_cast<char**>(ptr)[0] = free_slots;
      free_slots = reinterpret_cast<char*>(ptr);
    }
    
    void clear() {
      char* tmp = memory;
      while (tmp != nullptr) {
        char** ptr = reinterpret_cast<char**>(tmp);
        char* next_buffer = ptr[0];

        delete [] tmp;
        tmp = next_buffer;
      }
    }

    typeless_pool & operator=(const typeless_pool &pool) = delete;
    typeless_pool & operator=(typeless_pool &&pool) {
      type_size = pool.type_size;
      type_aligment = pool.type_aligment;
      block_size = pool.block_size;
      current_size = pool.current_size;
      memory = pool.memory;
      free_slots = pool.free_slots;

      pool.memory = nullptr;
      pool.current_size = 0;
      pool.free_slots = nullptr;
      
      return *this;
    }
  private:
    size_t type_size;
    size_t type_aligment;
    size_t block_size;
    size_t current_size;
    char* memory;
    char* free_slots;

    void allocate_block() {
      const size_t ptr_size = std::max(type_aligment, sizeof(char*));
      const size_t new_buffer_size = align_to(block_size + ptr_size, type_aligment);
      char* new_buffer = new char[new_buffer_size];

      char** tmp = reinterpret_cast<char**>(new_buffer);
      tmp[0] = memory;

      memory = new_buffer;
      current_size = 0;
    }
    
    void* allocate() {
      if (free_slots != nullptr) {
        auto ptr = reinterpret_cast<void*>(free_slots);
        auto tmp_slot = reinterpret_cast<char**>(free_slots);
        free_slots = tmp_slot[0];
        return ptr;
      }
      
      if (memory == nullptr || current_size + type_size > block_size) allocate_block();
      
      const size_t ptr_size = std::max(type_aligment, sizeof(char*));
      auto ptr = reinterpret_cast<void*>(memory+ptr_size+current_size);
      current_size += type_size;
      
      return ptr;
    }
  };
}

#endif //YACS_POOL_H
