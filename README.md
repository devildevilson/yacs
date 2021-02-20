# Yet another component system
Header-only c++11 entity component system   

Compile time type name as bonus for using c++17  
  
Example:  
```c++
#define YACS_IMPLEMENTATION // for some non-template code
#include "yacs.h"

// component size MUST be equal or more than sizeof(void*)

struct pickable {
  int attribute1;
  int attribute2;
};

struct graphics {
  int attribute1;
  int attribute2;
};

int main() {    
  yacs::world world;
  
  // entity creates this way
  auto ent = world.create_entity();
  // more precise control of memory pool memory size block
  world.create_allocator<pickable>(sizeof(pickable)*100);
  
  // adding new component to entity
  auto pickable_h = ent->add<pickable>();
  // getting component handle 
  auto h2 = ent->get<pickable>();
  // removing and destroying component
  ent->remove<pickable>();
  
  // world can be used as general data container
  auto graphics_h = world.create_component<graphics>();
  // we can set any manualy created components
  ent->set(graphics_h);
  // entity destruction automatically destroes graphics_h, thus we need to manualy unset component from entity
  ent->unset<graphics>();
  
  return 0;
}
```
  
More examples in "examples" folder.   

## TODO  

1. Better systems or remove systems ?
2. More examples
3. Tests
4. Benchmark
5. Multitheading
6. Remove id() from entity
7. ???

## License

Apache License 2.0
