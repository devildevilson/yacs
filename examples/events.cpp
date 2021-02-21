#define YACS_IMPLEMENTATION
#include "../yacs.h"
#include <iostream>
#include <vector>

#include <cassert>

struct event_data1 {
  int data;
};

struct event_data2 {
  void* data;
};

struct pod_component {
  uint32_t abc;
  uint64_t def;
  uint64_t ghi;
};

// yacs event, could be yacs::component_created, yacs::component_destroyed, yacs::entity_created, yacs::entity_destroyed
// see yacs_utils.h
class graphics_component_event : public yacs::event_subscriber<yacs::component_created<pod_component>> {
public:
  void receive(yacs::world* world, const yacs::component_created<pod_component> &event) override {
    (void)world;
    (void)event;
    std::cout << "Received component_created event" << "\n";
  }
};

// userdefined events
class event_receiver1 : public yacs::event_subscriber<event_data1> {
public:
  void receive(yacs::world* world, const event_data1 &event) override {
    (void)world;
    (void)event;
    std::cout << "Received event1" << "\n";
    assert(event.data == 24);
  }
};

class event_receiver2 : public yacs::event_subscriber<event_data2> {
public:
  void receive(yacs::world* world, const event_data2 &event) override {
    (void)world;
    (void)event;
    std::cout << "Received event2" << "\n";
    assert(event.data == nullptr);
  }
};

int main() {
  graphics_component_event gev;
  event_receiver1 er1;
  event_receiver2 er2;
  
  yacs::world world;
  world.subscribe(&gev);
  world.subscribe(&er1);
  world.subscribe(&er2);
  
  auto ent = world.create_entity();
  ent->add<pod_component>(); // component_created event
  
  event_data1 d1{
    24
  };
  
  event_data2 d2{
    nullptr
  };
  
  world.emit(d1); // event event_data1
  world.emit(d2); // event event_data2
  
  // if we get tired of events
  world.unsubscribe(&er1);
  
  world.emit(d1); // dont do anything
}
