#define YACS_IMPLEMENTATION
#include "../yacs.h"
#include <iostream>
#include <vector>

#include <cassert>

static size_t allocated_complex_component = 0;

struct memory_checker {
  ~memory_checker() {
    if (allocated_complex_component == 0) std::cout << "All complex components is freed" << "\n";
    else std::cout << "There are " << allocated_complex_component << " leaked components" << "\n";
  }
};

struct pod_component {
  uint32_t abc;
  uint64_t def;
  uint64_t ghi;
};

struct complex_component {
  char* array;
  std::vector<int> vector;
  
  complex_component(const size_t &size1, const size_t &size2) : array(new char[size1]), vector(size2, 0) {
    ++allocated_complex_component;
  }
  
  ~complex_component() {
    delete [] array;
    array = nullptr;
    
    --allocated_complex_component;
  }
};

const size_t entity_count = 1000;

int main() {
  memory_checker ch;
  
  yacs::world world;
  
  // create shared component
  auto handle = world.create_component<pod_component>();
  
  for (size_t i = 0; i < entity_count; ++i) {
    auto ent = world.create_entity();
    // set shared object to entity
    ent->set(handle);
    // using perfect forwarding to construct the object
    ent->add<complex_component>(i*10+1, i+1);
  }
  
  const size_t random_index = 352;
  auto itr = world.get_entities_view<complex_component>().begin();
  for (size_t i = 0; i < random_index; ++i) ++itr;
  
  std::cout << "Random complex_component destruction" << "\n";
  itr->remove<complex_component>();
  
  for (size_t i = 0; i < entity_count; ++i) {
    auto ent = world.create_entity();
    ent->set(handle);
    ent->add<complex_component>(i*10+1, i+1);
  }
  
  // component destroyes within entity's destruction, if we need continue using
  // the pod_component, we need to manually unset shared component from every entity
  for (auto &ent : world.get_entities_view<pod_component>()) {
    ent.unset<pod_component>();
  }
  
  return 0;
}
