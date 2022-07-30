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

namespace yacs {
  class world;
  class entity;

  class system {
  public:
    virtual ~system() = default;
    virtual void update(size_t time) = 0;
  };

  template<typename T>
  class event_subscriber {
  public:
    virtual ~event_subscriber() = default;
    virtual void receive(world* world, const T& event) = 0;
  };

  struct entity_created {
    handle<entity> ent;
  };

  struct entity_destroyed {
    handle<entity> ent;
  };

  template<typename T>
  struct component_created {
    handle<entity> ent;
    handle<T> component;
  };

  template<typename T>
  struct component_destroyed {
    handle<entity> ent;
    handle<T> component;
  };

  inline size_t castd(double d) {
    union { size_t l; double d; } local_u;
    local_u.d = d;
    return local_u.l;
  }

  inline double castl(size_t l) {
    union { size_t l; double d; } local_u;
    local_u.l = l;
    return local_u.d;
  }
}

#endif
