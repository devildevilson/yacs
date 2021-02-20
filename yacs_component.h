#ifndef YACS_COMPONENT_H
#define YACS_COMPONENT_H

#include <utility>

#ifndef _NDEBUG
#include <cassert>
#define ASSERT(expr) assert(expr)
#else
#define ASSERT(expr)
#endif

namespace yacs {
  template <typename T>
  struct component_storage {
    static size_t type_id;
    T obj;
    
    template <typename... Args>
    component_storage(Args&& ...args) : obj(std::forward<Args>(args)...) {}
    T* ptr() { return &obj; }
    const T* ptr() const { return &obj; }
  };
  
  template <typename T>
  struct event_container {
    static size_t type_id;
    T data;
    
    template <typename... Args>
    event_container(Args&& ...args) : data(std::forward<Args>(args)...) {}
    event_container(const T &data) : data(data) {}
    event_container(T &&data) : data(std::move(data)) {}
  };

  template <typename T>
  class component_handle {
  public:
    component_handle() : ptr(nullptr) {}
    component_handle(T* ptr) : ptr(ptr) {}
    ~component_handle() = default;
    component_handle(const component_handle &h) = default;
    component_handle(component_handle &&h) = default;
    component_handle & operator=(const component_handle &h) = default;
    component_handle & operator=(component_handle &&h) = default;
    
    bool valid() const { return ptr != nullptr; }
    T* get() { return ptr; }
    const T* get() const { return ptr; }
    T* operator->() { return ptr; }
    const T* operator->() const { return ptr; }
    bool operator==(const component_handle &handle) const { return ptr == handle.get(); }
    bool operator!=(const component_handle &handle) const { return ptr != handle.get(); }
  private:
    T* ptr;
  };

  template <typename T>
  class const_component_handle {
  public:
    const_component_handle() : ptr(nullptr) {}
    const_component_handle(const T* ptr) : ptr(ptr) {}
    ~const_component_handle() = default;
    const_component_handle(const const_component_handle &h) = default;
    const_component_handle(const_component_handle &&h) = default;
    const_component_handle & operator=(const const_component_handle &h) = default;
    const_component_handle & operator=(const_component_handle &&h) = default;
    
    bool valid() const { return ptr != nullptr; }
    const T* get() const { return ptr; }
    const T* operator->() const { return ptr; }
    bool operator==(const const_component_handle &handle) const { return ptr == handle.get(); }
    bool operator!=(const const_component_handle &handle) const { return ptr != handle.get(); }
  private:
    const T* ptr;
  };
  
  template <typename T>
  size_t component_storage<T>::type_id = SIZE_MAX;
  
  template <typename T>
  size_t event_container<T>::type_id = SIZE_MAX;
}

#endif //YACS_COMPONENT_H
