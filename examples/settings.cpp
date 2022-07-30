#include <iostream>
#include <vector>
#include <cassert>
#include <cstdint>

// control default memory pool memory block for components: block = count * sizeof(component_type)
#define YACS_DEFAULT_COMPONENTS_COUNT 200
// control default memory pool memory block for entities: block = count * sizeof(entity_type)
#define YACS_DEFAULT_ENTITY_COUNT 200
// control default memory pool memory block for additional entity component container: block = count * sizeof(entity_component_container_type)
#define YACS_DEFAULT_ENTITY_COMPONENT_CONTAINER_COUNT 200

// remove yacs asserts
#define _NDEBUG

#define YACS_IMPLEMENTATION
#include "../yacs.h"

struct component1 {
  uint64_t data;
};

struct component2 {
  uint64_t data;
};

struct component3 {
  uint64_t data;
};

struct component4 {
  uint64_t data;
};

struct component5 {
  uint64_t data;
};

int main() {
  yacs::world world;
  auto ent = world.create_entity();
  // use this to compute real component type size
  world.create_allocator<component1>(yacs::size_of<component1>()*10);
  world.create_allocator<component2>(yacs::size_of<component2>()*10);
  world.create_allocator<component3>(yacs::size_of<component3>()*10);
  world.create_allocator<component4>(yacs::size_of<component4>()*10);

  // does not create new component allocator
  ent->add<component4>();
  // creates new component allocator
  ent->add<component5>();
}
