#ifndef YACS_H
#define YACS_H

#include <vector>
#include <unordered_map>
#include <functional>
#include <cstdint>

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

#ifndef YACS_COMPONENT_TYPES_COUNT
#define YACS_COMPONENT_TYPES_COUNT 64
#endif

namespace yacs {
  class entity {
  public:
    static const size_t static_container_size = YACS_COMPONENT_TYPES_COUNT;
    using component_container_t = static_container<void*, static_container_size>;

    static size_t type;

    entity(const size_t &id, world* world);
    ~entity();

    template <typename T, typename ...Args>
    component_handle<T> add(Args&& ...args);

    template <typename T>
    bool remove();

    template <typename ...Types>
    bool has() const;

    template <typename T>
    component_handle<T> get();

    template <typename T>
    const_component_handle<T> get() const;
    
    template <typename T>
    bool set(component_handle<T> handle);
    
    template <typename T>
    bool unset();
    
    void* raw_get(const size_t &component_type) const;

    size_t id() const; // bad design and useless
    size_t components_count() const;
  private:
    size_t m_id;
    world* m_world;
  protected:
    bool has(const std::initializer_list<size_t> &list) const;
    
    void set_component(const size_t &type, void* comp);
    void remove_component(const size_t &type);
    void* get_component(const size_t &type) const;
    
    component_container_t m_components;
  };

  class world {
  public:
    template <typename T>
    class component_type_view {
    public:
      class iterator {
      public:
        using internal_type = phmap::flat_hash_set<void*>::iterator;
        using reference = T&;
        using pointer = T*;
        internal_type current;
        
        iterator(const internal_type &itr) : current(itr) {}
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
        
        reference operator*() { return *reinterpret_cast<pointer>(*current); }
        pointer operator->() { return reinterpret_cast<pointer>(*current); }
        
        friend bool operator==(const iterator& a, const iterator& b) { return a.current == b.current; }
        friend bool operator!=(const iterator& a, const iterator& b) { return a.current != b.current; }
      };
      
      class const_iterator {
      public:
        using internal_type = phmap::flat_hash_set<void*>::const_iterator;
        using reference = const T&;
        using pointer = const T*;
        internal_type current;
        
        const_iterator(const internal_type &itr) : current(itr) {}
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
        
        reference operator*() { return *reinterpret_cast<pointer>(*current); }
        pointer operator->() { return reinterpret_cast<pointer>(*current); }
        
        friend bool operator==(const const_iterator& a, const const_iterator& b) { return a.current == b.current; }
        friend bool operator!=(const const_iterator& a, const const_iterator& b) { return a.current != b.current; }
      };
      
      component_type_view(world* m_world);
      
      iterator begin();
      iterator end();
      const_iterator begin() const;
      const_iterator end() const;
    private:
      world* m_world;
    };
    
    template <typename... Args>
    class entities_view {
    public:
      class iterator {
      public:
        using internal_type = phmap::flat_hash_set<entity*>::iterator;
        using reference = entity&;
        using pointer = entity*;
        
        internal_type current;
        internal_type end;
        
        iterator(const internal_type &itr, const internal_type &end) : current(itr), end(end) {
          if (current == end) return;
          pointer ent = *current;
          while (!ent->has<Args...>()) {
            ++current;
            if (current == end) return;
            ent = *current;
          }
        }
        
        iterator(const iterator &other) = default;
        iterator(iterator &&other) = default;
        iterator & operator=(const iterator &other) = default;
        iterator & operator=(iterator &&other) = default;
        
        iterator & operator++() {
          if (current == end) return *this;
          
          pointer ent = nullptr;
          do {
            ++current;
            if (current == end) return *this;
            ent = *current;
          } while (!ent->has<Args...>());
          
          return *this;
        }
        
        iterator operator++(int) {
          auto cur = current;
          operator++();
          return cur;
        }
        
        reference operator*() { return **current; }
        pointer operator->() { return *current; }
        
        friend bool operator==(const iterator& a, const iterator& b) { return a.current == b.current; }
        friend bool operator!=(const iterator& a, const iterator& b) { return a.current != b.current; }
      };
      
      class const_iterator {
      public:
        using internal_type = phmap::flat_hash_set<entity*>::const_iterator;
        using reference = const entity&;
        using pointer = const entity*;
        
        internal_type current;
        internal_type end;
        
        const_iterator(const internal_type &itr, const internal_type &end) : current(itr), end(end) {}
        const_iterator(const const_iterator &other) = default;
        const_iterator(const_iterator &&other) = default;
        const_iterator & operator=(const const_iterator &other) = default;
        const_iterator & operator=(const_iterator &&other) = default;
        
        const_iterator & operator++() {
          if (current == end) return *this;
          
          pointer ent = nullptr;
          do {
            ++current;
            if (current == end) return *this;
            ent = *current;
          } while (!ent->has<Args...>());
          
          return *this;
        }
        
        const_iterator operator++(int) {
          auto cur = current;
          operator++();
          return cur;
        }
        
        reference operator*() { return **current; }
        pointer operator->() { return *current; }
        
        friend bool operator==(const const_iterator& a, const const_iterator& b) { return a.current == b.current; }
        friend bool operator!=(const const_iterator& a, const const_iterator& b) { return a.current != b.current; }
      };
      
      entities_view(world* m_world);
      
      iterator begin();
      iterator end();
      const_iterator begin() const;
      const_iterator end() const;
    private:
      world* m_world;
    };
    
    world();
    ~world();

    entity* create_entity();
    void destroy_entity(entity* ent);

    template <typename T>
    void create_allocator(const size_t &size);

    template <typename T, typename ...Args>
    component_handle<T> create_component(Args&& ...args);

    template <typename T>
    void destroy_component(component_handle<T> handle);

    void destroy_component(const size_t &type, void* storage);

    void register_system(system* system);
    void remove_system(system* system);

    template <typename T>
    void subscribe(event_subscriber<T>* sub);

    template <typename T>
    void unsubscribe(event_subscriber<T>* sub);

    template <typename T>
    void emit(const T &event);

    void update(YACS_SYSTEM_UPDATE);

    size_t size() const;

    template <typename T>
    size_t count_components() const;

    template <typename ...Types>
    size_t count_entities() const;
    
    // is there any chance to get component or entity using index for O(1) time?
//     template <typename T>
//     component_handle<T> get_component(const size_t &index);
//     entity* get_entity(const size_t &index);
    
    template <typename T>
    component_type_view<T> get_component_view();
    
    template <typename... Args>
    entities_view<Args...> get_entities_view();

    entity::component_container_t* create_entity_component_container();
    void destroy_entity_component_container(entity::component_container_t* cont);
  private:
    struct components_data {
      components_data(const size_t &type_size, const size_t &type_alignment, const size_t &size, const std::function<void(void*, typeless_pool&)> &destructor);
      components_data(components_data &&data) = default;
      ~components_data();
      
      components_data & operator=(components_data &&data) = default;

      template <typename T, typename ...Args>
      component_handle<T> create(Args&& ...args);

      template <typename T>
      void destroy(component_handle<T> handle);
      void destroy(void* storage);

      typeless_pool pool;
      phmap::flat_hash_set<void*> components;
      std::function<void(void*, typeless_pool&)> destructor;
    };
  
    size_t entity_id;
    typeless_pool m_entity_pool;
    typeless_pool m_entity_component_containers;
    phmap::flat_hash_set<entity*> m_entities;
    std::vector<struct components_data> m_components_data;

    phmap::flat_hash_set<system*> m_systems;
    std::vector<phmap::flat_hash_set<void*>> m_subscribers;
  };

  template <typename T, typename ...Args>
  component_handle<T> entity::add(Args&& ...args) {
    if (get_type_id<T>() != SIZE_MAX) {
      auto ptr = get_component(component_storage<T>::type_id);
      if (ptr != nullptr) return component_handle<T>(nullptr);
    }

    component_handle<T> handle = m_world->create_component<T>(std::forward<Args>(args)...);
    ASSERT(get_type_id<T>() != SIZE_MAX);
    ASSERT(handle.valid());
    set_component(get_type_id<T>(), handle.get());
    m_world->emit(component_created<T>{this, handle});

    return handle;
  }

  template <typename T>
  bool entity::remove() {
    if (get_type_id<T>() == SIZE_MAX) return false;

    auto comp = get_component(get_type_id<T>());
    if (comp == nullptr) return false;

    auto ptr = reinterpret_cast<component_storage<T>*>(comp);
    m_world->emit(component_destroyed<T>{this, component_handle<T>(ptr->ptr())});
    remove_component(get_type_id<T>());
    m_world->destroy_component(component_handle<T>(ptr->ptr()));
    return true;
  }

  template <typename ...Types>
  bool entity::has() const {
    const std::initializer_list<size_t> list = {get_type_id<Types>()...};
    return has(list);
  }

  template <typename T>
  component_handle<T> entity::get() {
    auto comp = get_component(get_type_id<T>());
    auto ptr = reinterpret_cast<component_storage<T>*>(comp);
    return component_handle<T>(ptr->ptr());
  }

  template <typename T>
  const_component_handle<T> entity::get() const {
    auto comp = get_component(get_type_id<T>());
    const auto* ptr = reinterpret_cast<const component_storage<T>*>(comp);
    return const_component_handle<T>(ptr->ptr());
  }
  
  template <typename T>
  bool entity::set(component_handle<T> handle) {
    if (has<T>()) return false;
    if (get_type_id<T>() == SIZE_MAX) return false;
    
    set_component(get_type_id<T>(), handle.get());
    return true;
  }
  
  template <typename T>
  bool entity::unset() {
    if (!has<T>()) return false;
    
    remove_component(get_type_id<T>());
    return true;
  }
  
  template <typename T>
  world::component_type_view<T>::component_type_view(world* m_world) : m_world(m_world) {
    const size_t type_id = get_type_id<T>();
    if (type_id == SIZE_MAX) throw std::runtime_error("Unknown component type");
  }
  
  template <typename T>
  typename world::component_type_view<T>::iterator world::component_type_view<T>::begin() {
    const size_t type_id = get_type_id<T>();
    return world::component_type_view<T>::iterator(m_world->m_components_data[type_id].components.begin());
  }
  
  template <typename T>
  typename world::component_type_view<T>::iterator world::component_type_view<T>::end() {
    const size_t type_id = get_type_id<T>();
    return world::component_type_view<T>::iterator(m_world->m_components_data[type_id].components.end());
  }
  
  template <typename T>
  typename world::component_type_view<T>::const_iterator world::component_type_view<T>::begin() const {
    const size_t type_id = get_type_id<T>();
    return world::component_type_view<T>::const_iterator(m_world->m_components_data[type_id].components.begin());
  }
  
  template <typename T>
  typename world::component_type_view<T>::const_iterator world::component_type_view<T>::end() const {
    const size_t type_id = get_type_id<T>();
    return world::component_type_view<T>::const_iterator(m_world->m_components_data[type_id].components.end());
  }
  
  template <typename... Args>
  world::entities_view<Args...>::entities_view(world* m_world) : m_world(m_world) {}
  
  template <typename... Args>
  typename world::entities_view<Args...>::iterator world::entities_view<Args...>::begin() {
    return world::entities_view<Args...>::iterator(m_world->m_entities.begin(), m_world->m_entities.end());
  }
  
  template <typename... Args>
  typename world::entities_view<Args...>::iterator world::entities_view<Args...>::end() {
    return world::entities_view<Args...>::iterator(m_world->m_entities.end(), m_world->m_entities.end());
  }
  
  template <typename... Args>
  typename world::entities_view<Args...>::const_iterator world::entities_view<Args...>::begin() const {
    return world::entities_view<Args...>::const_iterator(m_world->m_entities.begin(), m_world->m_entities.end());
  }
  
  template <typename... Args>
  typename world::entities_view<Args...>::const_iterator world::entities_view<Args...>::end() const {
    return world::entities_view<Args...>::const_iterator(m_world->m_entities.end(), m_world->m_entities.end());
  }

  template <typename T>
  void world::create_allocator(const size_t &size) {
    if (get_type_id<T>() < m_components_data.size()) return;

    component_storage<T>::type_id = m_components_data.size();
    m_components_data.emplace_back(sizeof(T), alignof(T), size, [] (void* ptr, typeless_pool &pool) {
      auto p = reinterpret_cast<component_storage<T>*>(ptr);
      pool.destroy(p);
    });
  }

  template <typename T, typename ...Args>
  component_handle<T> world::create_component(Args&& ...args) {
    if (get_type_id<T>() >= m_components_data.size()) {
      create_allocator<T>(YACS_DEFAULT_COMPONENTS_COUNT * sizeof(T));
      ASSERT(component_storage<T>::type_id < m_components_data.size());
    }
    
    const size_t poolIndex = get_type_id<T>();
    return m_components_data[poolIndex].create<T>(std::forward<Args>(args)...);
  }

  template <typename T>
  void world::destroy_component(component_handle<T> handle) {
    if (!handle.valid()) return;
    if (get_type_id<T>() >= m_components_data.size()) return;
    
    const size_t poolIndex = get_type_id<T>();
    m_components_data[poolIndex].destroy(handle);
  }

  template <typename T>
  void world::subscribe(event_subscriber<T>* sub) {
    if (event_container<T>::type_id >= m_subscribers.size()) {
      event_container<T>::type_id = m_subscribers.size();
      m_subscribers.emplace_back();
    }
    
    const size_t type_id = event_container<T>::type_id;
    auto &set = m_subscribers[type_id];
    set.emplace(reinterpret_cast<void*>(sub));
  }

  template <typename T>
  void world::unsubscribe(event_subscriber<T>* sub) {
    const size_t type_id = event_container<T>::type_id;
    if (type_id >= m_subscribers.size()) return;
    if (m_subscribers[type_id].empty()) return;

    auto &set = m_subscribers[type_id];
    set.erase(reinterpret_cast<void*>(sub));
  }

  template <typename T>
  void world::emit(const T &event) {
    const size_t type_id = event_container<T>::type_id;
    if (type_id >= m_subscribers.size()) return;
    if (m_subscribers[type_id].empty()) return;

    const auto &subs = m_subscribers[type_id];
    for (auto ptr : subs) {
      auto ev = reinterpret_cast<event_subscriber<T>*>(ptr);
      ev->receive(this, event);
    }
  }

  template <typename T>
  size_t world::count_components() const {
    const size_t type_id = get_type_id<T>();
    if (type_id >= m_components_data.size()) return 0;
    return m_components_data[type_id].components.size();
  }

  template <typename ...Types>
  size_t world::count_entities() const {
    size_t c = 0;
    for (auto entity : m_entities) {
      c += size_t(entity->has<Types...>());
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

  template <typename T, typename ...Args>
  component_handle<T> world::components_data::create(Args&& ...args) {
    auto ptr = pool.create<component_storage<T>>(std::forward<Args>(args)...);
    components.insert(ptr);
    return component_handle<T>(ptr->ptr());
  }

  template <typename T>
  void world::components_data::destroy(component_handle<T> handle) {
    auto ptr = reinterpret_cast<void*>(handle.get());
    components.erase(ptr);
    destructor(ptr, pool);
  }
}

#endif // YACS_H

#ifdef YACS_IMPLEMENTATION
namespace yacs {
  entity::entity(const size_t &id, world* world) : m_id(id), m_world(world), m_components(nullptr) {}
  entity::~entity() {
    size_t type_counter = 0;
    
    for (auto comp : m_components.array) {
      const size_t type_id = type_counter;
      ++type_counter;
      if (comp == nullptr) continue;
      m_world->destroy_component(type_id, comp);
    }
    
    auto comps = m_components.next;
    while (comps != nullptr) {
      for (auto comp : comps->array) {
        const size_t type_id = type_counter;
        ++type_counter;
        if (comp == nullptr) continue;
        m_world->destroy_component(type_id, comp);
      }

      auto old_comps = comps;
      comps = comps->next;
      m_world->destroy_entity_component_container(old_comps);
    }
  }

  bool entity::has(const std::initializer_list<size_t> &list) const {
    for (const auto &type : list) {
      if (get_component(type) == nullptr) return false;
    }

    return true;
  }
  
  void* entity::raw_get(const size_t &component_type) const {
    return get_component(component_type);
  }

  size_t entity::id() const { return m_id; }
  size_t entity::components_count() const { 
    size_t counter = 0;
    
    auto comps = &m_components;
    while (comps != nullptr) {
      for (auto comp : comps->array) {
        counter += size_t(comp != nullptr);
      }

      comps = comps->next;
    }
    
    return counter;
  }
  
  void entity::set_component(const size_t &type, void* comp) {
    ASSERT(type != SIZE_MAX);
    
    const size_t container_id    = type / static_container_size;
    const size_t container_index = type % static_container_size;
    
    auto ptr = &m_components;
    for (size_t i = 0; i < container_id; ++i) {
      ptr = ptr->next;
      if (ptr == nullptr) ptr = m_world->create_entity_component_container();
    }
    
    ptr->array[container_index] = comp;
  }
  
  void entity::remove_component(const size_t &type) {
    if (type == SIZE_MAX) return;
    
    const size_t container_id    = type / static_container_size;
    const size_t container_index = type % static_container_size;
    
    auto ptr = advance_static_container(&m_components, container_id);
    if (ptr == nullptr) return;
    ptr->array[container_index] = nullptr;
  }
  
  void* entity::get_component(const size_t &type) const {
    if (type == SIZE_MAX) return nullptr;
    
    const size_t container_id    = type / static_container_size;
    const size_t container_index = type % static_container_size;
    
    auto ptr = advance_static_container(&m_components, container_id);
    if (ptr == nullptr) return nullptr;
    return ptr->array[container_index];
  }

  size_t entity::type = SIZE_MAX;

  world::world() : 
    m_entity_pool(sizeof(entity), alignof(entity), sizeof(entity) * YACS_DEFAULT_ENTITY_COUNT),
    m_entity_component_containers(sizeof(entity::component_container_t), alignof(entity::component_container_t), sizeof(entity::component_container_t) * YACS_DEFAULT_ENTITY_COMPONENT_CONTAINER_COUNT)
  {}
  
  world::~world() {
    for (auto entity : m_entities) {
      m_entity_pool.destroy<class entity>(entity);
    }
  }

  entity* world::create_entity() {
    auto ent = m_entity_pool.create<entity>(m_entities.size(), this);
    m_entities.insert(ent);
    emit(entity_created{ent});

    return ent;
  }

  void world::destroy_entity(entity* ent) {
    emit(entity_destroyed{ent});
    m_entities.erase(ent);
    m_entity_pool.destroy(ent);
  }

  void world::destroy_component(const size_t &type, void* storage) {
    if (type >= m_components_data.size()) return;
    m_components_data[type].destroy(storage);
  }

  void world::register_system(system* system) {
    m_systems.insert(system);
  }

  void world::remove_system(system* system) {
    m_systems.erase(system);
  }

  void world::update(YACS_SYSTEM_FUNC) {
    for (auto system : m_systems) {
      system->update(YACS_SYSTEM_UPDATE_CALL);
    }
  }

  size_t world::size() const {
    return m_entities.size();
  }

  world::components_data::components_data(const size_t &type_size, const size_t &type_alignment, const size_t &size, const std::function<void(void*, typeless_pool&)> &destructor) : 
    pool(type_size, type_alignment, size), 
    destructor(destructor) 
  {}
  
  world::components_data::~components_data() {
    for (auto ptr : components) {
      destructor(ptr, pool);
    }
  }

  void world::components_data::destroy(void* storage) {
    components.erase(storage);
    destructor(storage, pool);
  }

  entity::component_container_t* world::create_entity_component_container() {
    return m_entity_component_containers.create<entity::component_container_t>(nullptr);
  }

  void world::destroy_entity_component_container(entity::component_container_t* cont) {
    m_entity_component_containers.destroy(cont);
  }
}
#endif // YACS_IMPLEMENTATION

