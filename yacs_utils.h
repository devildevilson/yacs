#ifndef YACS_UTILS_H
#define YACS_UTILS_H

#include <cstdint>
#include <string>

#if __cplusplus >= 201703L
#include <array>
#include <cctype>
#if defined(__GNUC__) && defined(__MINGW32__) && (__GNUC__ < 6)
extern "C" {
#include <ctype.h>
}
#endif // MinGW is on some stuff
#include <locale>
#endif

#include "yacs_component.h"

#ifndef YACS_UPDATE_TYPE
#define YACS_UPDATE_TYPE void* system = nullptr, float time = 0.0f
#define YACS_UPDATE_CALL system, time
#define YACS_SYSTEM_UPDATE float time = 0.0f
#define YACS_SYSTEM_FUNC float time
#define YACS_SYSTEM_UPDATE_CALL time
#endif // YACS_UPDATE_TYPE

namespace yacs {
  class world;
  class entity;
  
  template <typename T>
  constexpr size_t get_type_id() {
    return component_storage<T>::type_id;
  }
  
  class system {
  public:
    virtual ~system() = default;

    virtual void update(YACS_SYSTEM_UPDATE) = 0;
  };

  template<typename T>
  class event_subscriber {
  public:
    virtual ~event_subscriber() = default;
    virtual void receive(world* world, const T& event) = 0;
  };

  struct entity_created {
    entity* ent;
  };

  struct entity_destroyed {
    entity* ent;
  };

  template<typename T>
  struct component_created {
    entity* ent;
    component_handle<T> component;
  };

  template<typename T>
  struct component_destroyed {
    entity* ent;
    component_handle<T> component;
  };
  
#if __cplusplus >= 201703L
  inline constexpr std::array<std::string_view, 9> removals{ 
    { 
      "{anonymous}",
      "(anonymous namespace)",
      "public:",
      "private:",
      "protected:",
      "struct ",
      "class ",
      "`anonymous-namespace'",
      "`anonymous namespace'" 
    } 
  };

#if defined(__GNUC__) || defined(__clang__)
  inline std::string ctti_get_type_name_from_sig(std::string name) {
    // cardinal sins from MINGW
    size_t start = name.find_first_of('[');
    start = name.find_first_of('=', start);
    size_t end = name.find_last_of(']');
    
    if (end == std::string::npos) end = name.size();
    if (start == std::string::npos) start = 0;
    if (start < name.size() - 1) start += 1;
    
    name = name.substr(start, end - start);
    start = name.rfind("seperator_mark");
    if (start != std::string::npos) {
      name.erase(start - 2, name.length());
    }
    
    while (!name.empty() && isblank(name.front())) {
      name.erase(name.begin());
    }
    
    while (!name.empty() && isblank(name.back())) {
      name.pop_back();
    }

    for (std::size_t r = 0; r < removals.size(); ++r) {
      auto found = name.find(removals[r]);
      while (found != std::string::npos) {
        name.erase(found, removals[r].size());
        found = name.find(removals[r]);
      }
    }

    return name;
  }

  template <typename T, class seperator_mark = int>
  inline std::string internal_get_type_name() {
    return ctti_get_type_name_from_sig(__PRETTY_FUNCTION__);
  }
  
  template <typename T>
  std::string get_type_name() {
    return internal_get_type_name<T>();
  }

#elif defined(_MSC_VER)
  inline std::string ctti_get_type_name_from_sig(std::string name) {
    size_t start = name.find("get_type_name");
    
    if (start == std::string::npos) start = 0;
    else start += 13;
    if (start < name.size() - 1) start += 1;
    
    size_t end = name.find_last_of('>');
    if (end == std::string::npos) end = name.size();
    name = name.substr(start, end - start);
    
    if (name.find("struct", 0) == 0) name.replace(0, 6, "", 0);
    if (name.find("class", 0) == 0) name.replace(0, 5, "", 0);
    
    while (!name.empty() && isblank(name.front())) name.erase(name.begin());
    while (!name.empty() && isblank(name.back())) name.pop_back();

    for (std::size_t r = 0; r < removals.size(); ++r) {
      auto found = name.find(removals[r]);
      while (found != std::string::npos) {
        name.erase(found, removals[r].size());
        found = name.find(removals[r]);
      }
    }

    return name;
  }

  template <typename T>
  std::string get_type_name() {
    return ctti_get_type_name_from_sig(__FUNCSIG__);
  }
#else
#error Compiler not supported for demangling
#endif // compilers
#endif // __cplusplus >= 201703L

}

#endif
