#ifndef YACS_H
#define YACS_H

#include <vector>
#include <functional>
#include <cstdint>
#include <memory>

#include "yacs_entity.h"
#include "yacs_component.h"
#include "yacs_pool.h"
#include "yacs_utils.h"

// ugly
#include "include/parallel-hashmap/parallel_hashmap/phmap.h"

#ifndef YACS_DEFAULT_COMPONENTS_COUNT
#define YACS_DEFAULT_COMPONENTS_COUNT 100
#endif

#ifndef YACS_DEFAULT_ENTITY_COUNT
#define YACS_DEFAULT_ENTITY_COUNT 100
#endif

#ifndef YACS_DEFAULT_ENTITY_COMPONENT_CONTAINER_COUNT
#define YACS_DEFAULT_ENTITY_COMPONENT_CONTAINER_COUNT 100
#endif

// using flat_hash_map extensively, its slower than indexed array access,
// but faster than linear search in small sorted array

namespace yacs {
  // use this to compute real component type size
  template <typename T>
  size_t size_of() noexcept;

  class world {
    struct components_data;
  public:
    using entity_ptr_array_t = std::vector<std::pair<size_t, component_storage<entity>*>>;
    using component_ptr_array_t = std::vector<void*>;
    using component_data_container_t = phmap::flat_hash_map<size_t, std::unique_ptr<components_data>>;

    template <typename T>
    class component_type_view {
    public:
      using internal_type = component_ptr_array_t::iterator;

      class iterator {
      public:
        //using internal_type = phmap::flat_hash_set<void*>::iterator;
        using internal_type = component_ptr_array_t::iterator;
        using reference = T&;
        using pointer = T*;
        internal_type current;

        iterator(const internal_type &itr) noexcept : current(itr) {}
        iterator(const iterator &other) = default;
        iterator(iterator &&other) = default;
        iterator & operator=(const iterator &other) = default;
        iterator & operator=(iterator &&other) = default;

        iterator & operator++() {
          ++current;
          return *this;
        }

        iterator operator++(int) {
          auto cur = current;
          ++current;
          return cur;
        }

        //reference operator*() { return handle<T>(get_component_storage_id(), reinterpret_cast<T*>(*current)); }
        reference operator*() { return *reinterpret_cast<component_storage<T>*>(*current)->ptr(); }
        pointer operator->() { auto ptr = reinterpret_cast<component_storage<T>*>(*current); return ptr->ptr(); } // return handle<T>(get_component_storage_id(ptr), ptr);

        friend bool operator==(const iterator& a, const iterator& b) { return a.current == b.current; }
        friend bool operator!=(const iterator& a, const iterator& b) { return !(a == b); }
      };

      class const_iterator {
      public:
        //using internal_type = phmap::flat_hash_set<void*>::const_iterator;
        using internal_type = component_ptr_array_t::const_iterator;
        using reference = const T&;
        using pointer = const T*;
        internal_type current;

        const_iterator(const internal_type &itr) noexcept : current(itr) {}
        const_iterator(const const_iterator &other) = default;
        const_iterator(const_iterator &&other) = default;
        const_iterator & operator=(const const_iterator &other) = default;
        const_iterator & operator=(const_iterator &&other) = default;

        const_iterator & operator++() {
          ++current;
          return *this;
        }

        const_iterator operator++(int) {
          auto cur = current;
          ++current;
          return cur;
        }

        //reference operator*() { return handle<T>(get_component_storage_id(), reinterpret_cast<T*>(*current)); }
        reference operator*() const { return *reinterpret_cast<component_storage<T>*>(*current)->ptr(); }
        pointer operator->() const { auto ptr = reinterpret_cast<component_storage<T>*>(*current); return ptr->ptr(); } // return handle<T>(get_component_storage_id(ptr), ptr);

        friend bool operator==(const const_iterator& a, const const_iterator& b) { return a.current == b.current; }
        friend bool operator!=(const const_iterator& a, const const_iterator& b) { return !(a == b); }
      };

      component_type_view(world* m_world);

      iterator begin();
      iterator end();
      const_iterator begin() const;
      const_iterator end() const;
    private:
      internal_type m_begin;
      internal_type m_end;
    };

    template <typename... Args>
    class entities_view {
    public:
      using internal_type = entity_ptr_array_t::iterator;

      class iterator {
      public:
        //using internal_type = phmap::flat_hash_set<entity*>::iterator;
        using internal_type = entity_ptr_array_t::iterator;
        using reference = entity&;
        using pointer = handle<entity>;

        internal_type current;
        internal_type end;

        iterator(const internal_type &itr, const internal_type &end) noexcept : current(itr), end(end) {
          if (current == end) return;
          auto ent = current->second;
          while (ent != nullptr  && !ent->ptr()->has<Args...>()) {
            ++current;
            if (current == end) return;
            ent = current->second;
          }
        }

        iterator(const iterator &other) = default;
        iterator(iterator &&other) = default;
        iterator & operator=(const iterator &other) = default;
        iterator & operator=(iterator &&other) = default;

        iterator & operator++() {
          if (current == end) return *this;

          component_storage<entity>* ent = nullptr;
          do {
            ++current;
            if (current == end) return *this;
            ent = current->second;
          } while (ent != nullptr && !ent->ptr()->has<Args...>());

          return *this;
        }

        iterator operator++(int) {
          auto cur = current;
          operator++();
          return cur;
        }

        reference operator*() { return *current->second->ptr(); }
        pointer operator->() { auto ptr = current->second->ptr(); return handle<entity>(current->second->id, ptr); }

        friend bool operator==(const iterator& a, const iterator& b) { return a.current == b.current; }
        friend bool operator!=(const iterator& a, const iterator& b) { return !(a == b); }
      };

      class const_iterator {
      public:
        //using internal_type = phmap::flat_hash_set<entity*>::const_iterator;
        using internal_type = entity_ptr_array_t::const_iterator;
        using reference = const entity&;
        using pointer = handle<entity>;

        internal_type current;
        internal_type end;

        const_iterator(const internal_type &itr, const internal_type &end) noexcept : current(itr), end(end) {
          if (current == end) return;
          auto ent = current->second;
          while (ent != nullptr  && !ent->ptr()->has<Args...>()) {
            ++current;
            if (current == end) return;
            ent = current->second;
          }
        }

        const_iterator(const const_iterator &other) = default;
        const_iterator(const_iterator &&other) = default;
        const_iterator & operator=(const const_iterator &other) = default;
        const_iterator & operator=(const_iterator &&other) = default;

        const_iterator & operator++() {
          if (current == end) return *this;

          component_storage<entity>* ent = nullptr;
          do {
            ++current;
            if (current == end) return *this;
            ent = current->second;
          } while (ent != nullptr && !ent->ptr()->has<Args...>());

          return *this;
        }

        const_iterator operator++(int) {
          auto cur = current;
          operator++();
          return cur;
        }

        reference operator*() { return *current->second->ptr(); }
        pointer operator->() { auto ptr = current->second->ptr(); return handle<entity>(current->second->id, ptr); }

        friend bool operator==(const const_iterator& a, const const_iterator& b) { return a.current == b.current; }
        friend bool operator!=(const const_iterator& a, const const_iterator& b) { return !(a == b); }
      };

      entities_view(world* m_world);

      iterator begin();
      iterator end();
      const_iterator begin() const;
      const_iterator end() const;
    private:
      internal_type m_begin;
      internal_type m_end;
    };

    template <typename T>
    static size_t size_of();

    world() noexcept;
    ~world() noexcept;

    handle<entity> create_entity();
    void destroy_entity(handle<entity> ent);

    template <typename T>
    void create_allocator(const size_t &size);

    template <typename T, typename ...Args>
    handle<T> create_component(Args&& ...args);

    template <typename T>
    void destroy_component(handle<T> handle);

    void destroy_component(const size_t &type, void* storage);

    // remove systems
    void register_system(system* system);
    void remove_system(system* system);

    template <typename T>
    void subscribe(event_subscriber<T>* sub);

    template <typename T>
    void unsubscribe(event_subscriber<T>* sub);

    template <typename T>
    void emit(const T &event);

    void update(const size_t &time);

    size_t size() const;

    template <typename T>
    size_t count_components() const;

    template <typename ...Types>
    size_t count_entities() const;

    template <typename T>
    handle<T> get_component(const size_t &index) const;
    handle<entity> get_entity(const size_t &id) const;

    template <typename T>
    component_type_view<T> get_component_view();

    template <typename... Args>
    entities_view<Args...> get_entities_view();

    //size_t find_type_index(const size_t &type) const;
  private:
    struct components_data {
      using destructor_t = std::function<void(void*, typeless_pool&)>;

      components_data(const size_t &type_size, const size_t &type_alignment, const size_t &size, const destructor_t &f);
      components_data(const components_data &data) noexcept = delete;
      components_data(components_data &&data) noexcept = default;
      components_data & operator=(const components_data &data) noexcept = delete;
      components_data & operator=(components_data &&data) noexcept = default;
      ~components_data();

      template <typename T, typename ...Args>
      handle<T> create(Args&& ...args);

      template <typename T>
      void destroy(handle<T> handle);
      void destroy(void* storage);

      size_t current_id;
      typeless_pool pool;
      //phmap::flat_hash_set<void*> components;
      component_ptr_array_t components;
      destructor_t destructor;
    };

    template <typename T>
    component_data_container_t::iterator create_allocator_raw(const size_t &size);

    typeless_pool m_entity_pool;
    entity_ptr_array_t m_entities;
    component_data_container_t m_components_data;
    std::vector<system*> m_systems;
    //std::vector<phmap::flat_hash_set<void*>> m_subscribers;
    phmap::flat_hash_map<size_t, phmap::flat_hash_set<void*>> m_subscribers;
  };

  // наверное имеет смысл запомнить id у компонента, компонент может быть неаккуратно удален
  template <typename T, typename ...Args>
  handle<T> entity::add(Args&& ...args) {
    auto ptr = get_component(type_id<T>());
    if (ptr != nullptr) return handle<T>();

    auto h = m_world->create_component<T>(std::forward<Args>(args)...);
    YACS_ASSERT(h.valid());
    set_component(type_id<T>(), h.get());
    m_world->emit(component_created<T>{handle<entity>(get_component_storage_id(this), this), h});
    return h;
  }

  template <typename T>
  bool entity::remove() {
    auto comp = get_component(type_id<T>());
    if (comp == nullptr) return false;

    auto ptr = reinterpret_cast<component_storage<T>*>(comp);
    m_world->emit(component_destroyed<T>{handle<entity>(get_component_storage_id(this), this), handle<T>(ptr->id, ptr->ptr())});
    remove_component(type_id<T>());
    m_world->destroy_component(handle<T>(ptr->id, ptr->ptr()));
    return true;
  }

  template <typename... Types>
  bool entity::has() const noexcept {
    const std::initializer_list<size_t> list = { type_id<Types>()... };
    return has(list);
  }

  template <typename T>
  handle<T> entity::get() const noexcept {
    auto comp = get_component(type_id<T>());
    if (comp == nullptr) return handle<T>();
    auto ptr = reinterpret_cast<component_storage<T>*>(comp);
    return handle<T>(ptr->id, ptr->ptr());
  }

  template <typename T>
  bool entity::set(handle<T> handle) {
    if (has<T>()) return false;
    //if (get_type_id<T>() == SIZE_MAX) return false;

    set_component(type_id<T>(), handle.get());
    return true;
  }

  template <typename T>
  bool entity::unset() {
    if (!has<T>()) return false;

    remove_component(type_id<T>());
    return true;
  }

  template <typename T>
  size_t size_of() { return sizeof(component_storage<T>); }

  template <typename T>
  world::component_type_view<T>::component_type_view(world* m_world) {
    const size_t type = type_id<T>();
    //const size_t index = find_type_index(type_id);
    const auto itr = m_world->m_components_data.find(type);
    m_begin = itr->second->components.begin();
    m_end = itr->second->components.end();
  }

  template <typename T>
  typename world::component_type_view<T>::iterator world::component_type_view<T>::begin() {
    return world::component_type_view<T>::iterator(m_begin);
  }

  template <typename T>
  typename world::component_type_view<T>::iterator world::component_type_view<T>::end() {
    return world::component_type_view<T>::iterator(m_end);
  }

  template <typename T>
  typename world::component_type_view<T>::const_iterator world::component_type_view<T>::begin() const {
    return world::component_type_view<T>::const_iterator(m_begin);
  }

  template <typename T>
  typename world::component_type_view<T>::const_iterator world::component_type_view<T>::end() const {
    return world::component_type_view<T>::const_iterator(m_end);
  }

  template <typename... Args>
  world::entities_view<Args...>::entities_view(world* m_world) : m_begin(m_world->m_entities.begin()), m_end(m_world->m_entities.end()) {}

  template <typename... Args>
  typename world::entities_view<Args...>::iterator world::entities_view<Args...>::begin() {
    return world::entities_view<Args...>::iterator(m_begin, m_end);
  }

  template <typename... Args>
  typename world::entities_view<Args...>::iterator world::entities_view<Args...>::end() {
    return world::entities_view<Args...>::iterator(m_end, m_end);
  }

  template <typename... Args>
  typename world::entities_view<Args...>::const_iterator world::entities_view<Args...>::begin() const {
    return world::entities_view<Args...>::const_iterator(m_begin, m_end);
  }

  template <typename... Args>
  typename world::entities_view<Args...>::const_iterator world::entities_view<Args...>::end() const {
    return world::entities_view<Args...>::const_iterator(m_end, m_end);
  }

  template <typename T>
  void world::create_allocator(const size_t &size) { create_allocator_raw<T>(size); }

  template <typename T, typename ...Args>
  handle<T> world::create_component(Args&& ...args) {
    auto itr = m_components_data.find(type_id<T>());
    if (itr == m_components_data.end()) {
      itr = create_allocator_raw<T>(YACS_DEFAULT_COMPONENTS_COUNT * sizeof(component_storage<T>));
    }

    return itr->second-> template create<T>(std::forward<Args>(args)...);
  }

  template <typename T>
  void world::destroy_component(handle<T> h) {
    if (!h.valid()) return;
    auto itr = m_components_data.find(type_id<T>());
    if (itr == m_components_data.end()) return;
    itr->second->destroy(h);
  }

  template <typename T>
  void world::subscribe(event_subscriber<T>* sub) {
    m_subscribers[type_id<T>()].insert(reinterpret_cast<void*>(sub));
  }

  template <typename T>
  void world::unsubscribe(event_subscriber<T>* sub) {
    auto itr = m_subscribers.find(type_id<T>());
    if (itr == m_subscribers.end()) return;
    itr->second.erase(sub);
  }

  template <typename T>
  void world::emit(const T &event) {
    const auto itr = m_subscribers.find(type_id<T>());
    if (itr == m_subscribers.end()) return;

    for (auto ptr : itr->second) {
      auto ev = reinterpret_cast<event_subscriber<T>*>(ptr);
      ev->receive(this, event);
    }
  }

  template <typename T>
  size_t world::count_components() const {
    const auto itr = m_components_data.find(type_id<T>());
    if (itr == m_components_data.end()) return 0;
    return itr->second.components.size();
  }

  template <typename ...Types>
  size_t world::count_entities() const {
    size_t c = 0;
    for (const auto &p : m_entities) {
      if (p.second == nullptr) continue;
      c += size_t(p.second->ptr()->has<Types...>());
    }

    return c;
  }

  template <typename T>
  world::component_type_view<T> world::get_component_view() {
    return world::component_type_view<T>(this);
  }

  template <typename... Args>
  world::entities_view<Args...> world::get_entities_view() {
    return world::entities_view<Args...>(this);
  }

  template <typename T>
  world::component_data_container_t::iterator world::create_allocator_raw(const size_t &size) {
    if (const auto itr = m_components_data.find(type_id<T>()); itr != m_components_data.end()) return m_components_data.end();

    return m_components_data.emplace(type_id<T>(), std::make_unique<world::components_data>(sizeof(component_storage<T>), alignof(component_storage<T>), size, [] (void* ptr, typeless_pool &pool) {
      auto p = reinterpret_cast<component_storage<T>*>(ptr);
      pool.destroy(p);
    })).first;
  }

  template <typename T, typename ...Args>
  handle<T> world::components_data::create(Args&& ...args) {
    auto ptr = pool.create<component_storage<T>>(current_id, std::forward<Args>(args)...);
    components.push_back(ptr);
    current_id++;
    return handle<T>(ptr->id, ptr->ptr());
  }

  template <typename T>
  void world::components_data::destroy(handle<T> h) {
    auto ptr = reinterpret_cast<void*>(h.get());
    destroy(ptr);
  }
}

#endif // YACS_H

#ifdef YACS_IMPLEMENTATION
namespace yacs {
  entity::entity(world* world) noexcept : m_world(world) {}
  entity::~entity() noexcept {
    // size_t type_counter = 0;
    //
    // for (auto comp : m_components.array) {
    //   const size_t type_id = type_counter;
    //   ++type_counter;
    //   if (comp == nullptr) continue;
    //   m_world->destroy_component(type_id, comp);
    // }
    //
    // auto comps = m_components.next;
    // while (comps != nullptr) {
    //   for (auto comp : comps->array) {
    //     const size_t type_id = type_counter;
    //     ++type_counter;
    //     if (comp == nullptr) continue;
    //     m_world->destroy_component(type_id, comp);
    //   }
    //
    //   auto old_comps = comps;
    //   comps = comps->next;
    //   m_world->destroy_entity_component_container(old_comps);
    // }

    for (const auto &pair : m_components) {
      m_world->destroy_component(pair.first, pair.second);
    }
  }

  bool entity::has(const std::initializer_list<size_t> &list) const noexcept {
    for (const size_t &type : list) {
      if (!raw_has(type)) return false;
    }

    return true;
  }

  void* entity::raw_get(const size_t &component_type) const noexcept {
    return get_component(component_type);
  }

  size_t entity::id() const noexcept { return get_component_storage_id(this); }
  size_t entity::components_count() const noexcept {
    return m_components.size();
  }

  void entity::set_component(const size_t &type, void* comp) noexcept {
    m_components[type] = comp;
  }

  void entity::remove_component(const size_t &type) noexcept {
    auto itr = m_components.find(type);
    if (itr == m_components.end()) return;
    m_components._erase(itr);
  }

  void* entity::get_component(const size_t &type) const noexcept {
    const auto itr = m_components.find(type);
    return itr != m_components.end() ? itr->second : nullptr;
  }

  bool entity::raw_has(const size_t &type) const noexcept {
    const auto itr = m_components.find(type);
    return itr != m_components.end();
  }

  world::world() noexcept :
    m_entity_pool(sizeof(component_storage<entity>) * YACS_DEFAULT_ENTITY_COUNT, sizeof(component_storage<entity>), alignof(component_storage<entity>))
    //m_entity_component_containers(sizeof(entity::component_container_t), alignof(entity::component_container_t), sizeof(entity::component_container_t) * YACS_DEFAULT_ENTITY_COMPONENT_CONTAINER_COUNT)
  {}

  world::~world() noexcept {
    for (const auto &pair : m_entities) {
      m_entity_pool.destroy(pair.second);
    }
  }

  static size_t make_id(const uint32_t index, const uint32_t counter) {
    return (size_t(counter) << 32) | size_t(index);
  }

  handle<entity> world::create_entity() {
    size_t index = 0;
    for (; index < m_entities.size() && m_entities[index].second != nullptr; ++index) {}
    if (index == m_entities.size()) m_entities.emplace_back(0, nullptr);
    static_assert(sizeof(size_t) == 8);
    //const entity::id id(index, m_entities[index].first);
    const size_t id = make_id(index, m_entities[index].first);
    auto ent = m_entity_pool.create<component_storage<entity>>(id, this);
    m_entities[index].second = ent;
    const handle<entity> h(id, ent->ptr());
    emit(entity_created{h});
    return h;
  }

  void world::destroy_entity(handle<entity> ent) {
    if (!ent.valid()) return;
    emit(entity_destroyed{ent});
    const size_t id = get_component_storage_id(ent.get());
    const size_t index = size_t(uint32_t(id));
    //m_entities.erase(ent);
    assert(index < m_entities.size());
    m_entity_pool.destroy(m_entities[index].second);
    m_entities[index].first+=1;
    m_entities[index].second = nullptr;
  }

  void world::destroy_component(const size_t &type, void* storage) {
    // const size_t index = find_type_index(type);
    // if (m_components_data[index].second == nullptr) return;
    // m_components_data[index].second->destroy(storage);
    const auto itr = m_components_data.find(type);
    if (itr == m_components_data.end()) return;
    itr->second->destroy(storage);
  }

  // тут неправильно
  void world::register_system(system* system) {
    m_systems.push_back(system);
  }

  void world::remove_system(system* system) {
    m_systems.erase(std::find(m_systems.begin(), m_systems.end(), system));
  }

  void world::update(const size_t &time) {
    for (auto system : m_systems) {
      system->update(time);
    }
  }

  size_t world::size() const {
    return m_entities.size();
  }

  // size_t world::find_type_index(const size_t &type) const {
  //   size_t index = type % components_type_count;
  //   size_t counter = 1;
  //   while (counter < components_type_count && m_components_data[index].first != type && m_components_data[index].second != nullptr) {
  //     index = (index + counter) % components_type_count;
  //     ++counter;
  //   }
  //
  //   return index;
  // }

  world::components_data::components_data(const size_t &type_size, const size_t &type_alignment, const size_t &size, const destructor_t &destructor) :
    current_id(0),
    pool(size, type_size, type_alignment),
    destructor(destructor)
  {}

  world::components_data::~components_data() {
    for (auto ptr : components) {
      destructor(ptr, pool);
    }
  }

  void world::components_data::destroy(void* storage) {
    // auto itr = components.find(storage);
    // if (itr == components.end()) return;
    // components._erase(itr);
    // destructor(storage, pool);
    for (size_t i = 0; i < components.size(); ++i) {
      if (components[i] != storage) continue;
      std::swap(components[i], components.back());
      components.pop_back();
      destructor(storage, pool);
      return;
    }
  }

  // entity::component_container_t* world::create_entity_component_container() {
  //   return m_entity_component_containers.create<entity::component_container_t>(nullptr);
  // }
  //
  // void world::destroy_entity_component_container(entity::component_container_t* cont) {
  //   m_entity_component_containers.destroy(cont);
  // }

  typeless_pool::typeless_pool(const size_t size, const size_t piece_size, const size_t piece_aligment) noexcept :
    m_size(align_to(size, piece_aligment)), m_piece_size(piece_size), m_piece_aligment(piece_aligment), m_current(0), m_memory(nullptr), m_free_mem(nullptr)
  {
    assert(m_size >= sizeof(void*)*2);
    assert(m_piece_size >= sizeof(void*));
  }

  typeless_pool::~typeless_pool() noexcept { clear(); }

  typeless_pool::typeless_pool(typeless_pool &&move) noexcept :
    m_size(move.m_size), m_piece_size(move.m_piece_size), m_piece_aligment(move.m_piece_aligment), m_current(move.m_current), m_memory(move.m_memory), m_free_mem(move.m_free_mem)
  {
    move.m_memory = nullptr;
    move.m_free_mem = nullptr;
  }

  typeless_pool & typeless_pool::operator=(typeless_pool &&move) noexcept {
    m_size = move.m_size;
    m_piece_size = move.m_piece_size;
    m_piece_aligment = move.m_piece_aligment;
    m_current = move.m_current;
    m_memory = move.m_memory;
    m_free_mem = move.m_free_mem;
    move.m_memory = nullptr;
    move.m_free_mem = nullptr;
    return *this;
  }

  void* typeless_pool::allocate() {
    if (m_free_mem != nullptr) {
      auto ptr = m_free_mem;
      m_free_mem = reinterpret_cast<char**>(m_free_mem)[0];
      return ptr;
    }

    if (m_memory == nullptr || (m_current + m_piece_size > m_size)) allocate_block();
    auto ptr = &m_memory[m_current];
    m_current += m_piece_size;
    return ptr;
  }

  void typeless_pool::free(void* block) noexcept {
    reinterpret_cast<void**>(block)[0] = m_free_mem;
    m_free_mem = block;
  }

  size_t typeless_pool::size() const noexcept {
    size_t summ = 0;
    auto mem = m_memory;
    while (mem != nullptr) {
      summ += m_size;
      mem = reinterpret_cast<char**>(mem)[0];
    }

    return summ;
  }

  size_t typeless_pool::block_size() const noexcept { return m_size; }
  size_t typeless_pool::piece_size() const noexcept { return m_piece_size; }
  size_t typeless_pool::piece_aligment() const noexcept { return m_piece_aligment; }

  void typeless_pool::allocate_block() {
    const size_t offset = std::max(m_piece_aligment, sizeof(void*));
    const size_t final_size = offset + m_size;
    auto new_mem = new (std::align_val_t{m_piece_aligment}) char[final_size];
    assert(new_mem != nullptr);
    reinterpret_cast<char**>(new_mem)[0] = m_memory;
    m_memory = new_mem;
    m_current = offset;
  }

  void typeless_pool::clear() noexcept {
    auto mem = m_memory;
    while (mem != nullptr) {
      auto next = reinterpret_cast<char**>(mem)[0];
      ::operator delete[] (mem, std::align_val_t{m_piece_aligment});
      mem = next;
    }
  }
}
#endif // YACS_IMPLEMENTATION
