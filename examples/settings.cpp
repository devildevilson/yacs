// control default memory pool memory block for components: block = count * sizeof(component_type)
#define YACS_DEFAULT_COMPONENTS_COUNT 200
// control default memory pool memory block for entities: block = count * sizeof(entity_type)
#define YACS_DEFAULT_ENTITY_COUNT 200
// control default memory pool memory block for additional entity component container: block = count * sizeof(entity_component_container_type)
#define YACS_DEFAULT_ENTITY_COMPONENT_CONTAINER_COUNT 200
// control entity component container size, if component TYPES count exceed this amount 
// entity allocates additional component containers
#define YACS_COMPONENT_TYPES_COUNT 3

// remove yacs asserts
#define _NDEBUG

#define YACS_IMPLEMENTATION
#include "../yacs.h"
#include <iostream>
#include <vector>

#include <cassert>
#include <cstdint>

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

int main() {
  yacs::world world;
  auto ent = world.create_entity();
  world.create_allocator<component1>(sizeof(component1)*10); // type id == 0
  world.create_allocator<component2>(sizeof(component2)*10); // type id == 1
  world.create_allocator<component3>(sizeof(component3)*10); // type id == 2
  world.create_allocator<component4>(sizeof(component4)*10); // type id == 3
  
  // allocates new component container
  // its better to avoid this by setting YACS_COMPONENT_TYPES_COUNT to big number
  ent->add<component4>();
}
