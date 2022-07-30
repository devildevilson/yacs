#ifndef YACS_COMPONENT_H
#define YACS_COMPONENT_H

#include <utility>
#include <string>
#include <stdexcept>
#include "yacs_type_traits.h"

#ifndef _NDEBUG
#include <cassert>
#define YACS_ASSERT(expr) assert(expr)
#else
#define YACS_ASSERT(expr)
#endif

namespace yacs {
  class entity;

  template <typename T>
  struct component_storage {
    T obj;
    size_t id;

    template <typename... Args>
    component_storage(const size_t &id, Args&& ...args) : obj(std::forward<Args>(args)...), id(id) {}
    ~component_storage() { id = SIZE_MAX; }
    T* ptr() { return &obj; }
    const T* ptr() const { return &obj; }
  };

  template <typename T>
  size_t get_component_storage_id(const T* comp) {
    //static_assert(offsetof(component_storage<T>, obj) == 0);
    auto mem = reinterpret_cast<const component_storage<T>*>(comp);
    return mem->id;
  }

  template <typename T>
  struct event_container {
    T data;

    template <typename... Args>
    event_container(Args&& ...args) : data(std::forward<Args>(args)...) {}
    event_container(const T &data) : data(data) {}
    event_container(T &&data) : data(std::move(data)) {}
  };

  template <typename T>
  class handle {
  public:
    handle() noexcept : id(SIZE_MAX), ptr(nullptr) {}
    handle(const size_t &id, T* ptr) noexcept : id(id), ptr(ptr) {}
    ~handle() noexcept = default;
    handle(const handle &copy) noexcept = default;
    handle(handle &&move) noexcept = default;
    handle & operator=(const handle &copy) noexcept = default;
    handle & operator=(handle &&move) noexcept = default;

    bool valid() const noexcept { return id != SIZE_MAX && id == get_component_storage_id(ptr); }
          T* get()       noexcept { return valid() ? ptr : nullptr; }
    const T* get() const noexcept { return valid() ? ptr : nullptr; }
          T* operator->()       { not_valid_throw(); return ptr; }
    const T* operator->() const { not_valid_throw(); return ptr; }
          T & operator*()       { not_valid_throw(); return *ptr; }
    const T & operator*() const { not_valid_throw(); return *ptr; }
    bool operator==(const handle &handle) const noexcept { return ptr == handle.ptr && id == handle.id; }
    bool operator!=(const handle &handle) const noexcept { return !operator==(handle); }
    void not_valid_throw() const { if (!valid()) throw std::runtime_error("Handle contains invalid data of type '" + std::string(type_name<T>()) + "'"); }
  private:
    size_t id;
    T* ptr;
  };
}

#endif //YACS_COMPONENT_H
