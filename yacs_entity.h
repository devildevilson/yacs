#ifndef YACS_ENTITY_H
#define YACS_ENTITY_H

#include <cstddef>
#include <cstdint>
// ugly
#include "include/parallel-hashmap/parallel_hashmap/phmap.h"
#include "yacs_component.h"

namespace yacs {
  class world;

  class entity {
  public:
    using component_container_t = phmap::flat_hash_map<size_t, void*>; // мож так оставить?

    struct id {
      uint32_t index;
      uint32_t counter;

      id() noexcept = default;
      inline id(uint32_t index, uint32_t counter) noexcept : index(index), counter(counter) {}
      id(const id &copy) noexcept = default;
      id(id &&move) noexcept = default;
      id & operator=(const id &copy) noexcept = default;
      id & operator=(id &&move) noexcept = default;
      inline bool operator==(const id &b) const noexcept { return index == b.index && counter == b.counter; }
      inline bool operator!=(const id &b) const noexcept { return !operator==(b); }
      size_t make_id() const noexcept { return (size_t(counter) << 32) | size_t(index); }
    };

    entity(world* world) noexcept;
    ~entity() noexcept;

    entity(const entity &copy) noexcept = delete;
    entity(entity &&move) noexcept = default;
    entity & operator=(const entity &copy) noexcept = delete;
    entity & operator=(entity &&move) noexcept = default;

    template <typename T, typename ...Args>
    handle<T> add(Args&& ...args);

    template <typename T>
    bool remove();

    template <typename ...Types>
    bool has() const noexcept;

    template <typename T>
    handle<T> get() const noexcept;

    template <typename T>
    bool set(handle<T> h);

    template <typename T>
    bool unset();

    void* raw_get(const size_t &component_type) const noexcept;

    size_t id() const noexcept;
    size_t components_count() const noexcept;
  private:
    //size_t m_id;
    world* m_world;
  protected:
    bool has(const std::initializer_list<size_t> &list) const noexcept;
    void set_component(const size_t &type, void* comp) noexcept;
    void remove_component(const size_t &type) noexcept;
    void* get_component(const size_t &type) const noexcept;
    size_t find_type_index(const size_t &type) const noexcept;
    bool raw_has(const size_t &type) const noexcept;

    component_container_t m_components;
  };
}

#endif
