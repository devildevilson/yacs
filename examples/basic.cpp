#define YACS_IMPLEMENTATION
#include "../yacs.h"
#include <iostream>

#include <cassert>

// component size must be equal or more than sizeof(char*)

namespace abc {
  struct transform {
    float x, y, z;
    
    ~transform() {
      std::cout << "transform destroyed" << "\n";
    }
  };
}

struct pickable {
  int attribute1;
  int attribute2;
};

struct graphics {
  int image_id1;
  int image_id2;
};

class graphics_component_event : public yacs::event_subscriber<yacs::component_created<graphics>> {
public:
  void receive(yacs::world* world, const yacs::component_created<graphics> &event) override {
    (void)world;
    (void)event;
    std::cout << "Received component_created event" << "\n";
  }
};

std::string replace(const std::string &str, const std::string &target, const std::string &replace_to) {
  std::string copy = str;
  size_t pos = copy.find(target, 0);
  while (pos != std::string::npos) {
    copy.replace(pos, target.length(), replace_to);
    pos = copy.find(target, pos+target.length());
  }

  return copy;
}

int main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  
  graphics_component_event ev;
  
  yacs::world world;
  
  world.create_allocator<graphics>(sizeof(graphics)*100);
  assert(yacs::get_type_id<graphics>() == 0);
  assert(yacs::get_type_id<abc::transform>() == SIZE_MAX);
  
  world.subscribe(&ev);
  
  auto ent = world.create_entity();
  auto trans_h = ent->add<abc::transform>();
  auto pickable_h = ent->add<pickable>();
  auto graphics_h = ent->add<graphics>();
  
  trans_h->x = 234.0f;
  pickable_h->attribute1 = 235;
  graphics_h->image_id1 = 74574;
  
  auto ent2 = world.create_entity();
  ent2->add<abc::transform>();
  ent2->add<pickable>();
  ent2->add<graphics>();
  
  const size_t count = world.count_entities<pickable, graphics>();
  assert(count == 2);
  
  // forward iterators
  for (auto itr = world.get_entities_view<pickable, graphics>().begin(); itr != world.get_entities_view<pickable, graphics>().end(); ++itr) {
    const auto &ent = *itr;
    auto h = ent.get<pickable>();
    assert(h.valid());
  }
  
  size_t counter = 0;
  for (const auto &ent : world.get_entities_view<pickable, graphics>()) {
    auto h = ent.get<pickable>();
    assert(h.valid());
    
    ++counter;
  }
  
  assert(counter == 2);
  
  // is reference to component is better than component handle?
  for (const auto &comp : world.get_component_view<pickable>()) {
    (void)comp;
  }
  
  assert(ent->get<abc::transform>().valid());
  assert(ent->get<abc::transform>()->x == 234.0f);
  assert(ent->get<pickable>().valid());
  assert(ent->get<pickable>()->attribute1 == 235);
  assert(ent->get<graphics>().valid());
  assert(ent->get<graphics>()->image_id1 == 74574);
  
  ent->remove<abc::transform>();
  ent->remove<pickable>();
  ent->remove<graphics>();
  
  assert(!ent->get<abc::transform>().valid());
  assert(!ent->get<pickable>().valid());
  assert(!ent->get<graphics>().valid());
  
  world.destroy_entity(ent);
  
  // bonus content
#if __cplusplus >= 201703L
  const auto fine_str = replace(yacs::get_type_name<abc::transform>(), "::", "_");
  std::cout << "transform name " << fine_str << "\n";
#endif
  
  // autorelease all entites on world destruction 

  return 0;
}
