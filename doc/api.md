# Initialization of a Module

A module wraps the Root Component creation for all sessions. These sessions
finally intercept all IPC and service requests. The `base` module implements the
default checkpoint/restore. If the checkpoint/restore mechanism is extended,
another module will be implemented. The user has to choose one module in the
beginning.

```C++
\* Rtcr includes */
#include <rtcr/child.h>
#include <rtcr/module_factory.h>
#include <rtcr/init_module.h>
#include <rtcr_serializer/serializer.h>
```

```C++
Base_module module(env, heap);
```

It is possible to to initialize the module based on the module name.

```C++
Init_module &module = *Module_factory::get("base")->create(env, heap);
```



# Child Creation

```C++
Child sheep (env, heap, "sheep_counter", module.services(), parent_servies);
```

Multiple child are supported.


# Checkpointing
All childs are paused, checkpointed and resumed in parallel.

```C++
module.checkpoint();
```

The last checkpointed state can be examined:

```C++
Child_info *sheep_info = module.child_info("sheep_counter);
Genode::log(*sheep_info);
```

# Serialization

The `Serializer` class compress the last checkpoint of `sheep` and provides it
as dataspace `ds_cap`. The actual size of data in the dataspace is stored in the
variable `size`. 

```C++
Serializer s(env, heap);

Genode::size_t size;
Genode::List<Child_info> *child_infos = module.child_info();
Genode::Ram_dataspace_capability ds_cap = s.serialize(child_infos, &size);
```

Parsing a serialized state from a dataspace is also simple:

```C++
child_infos = serializer.parse(ds_cap);
Genode::log(*child_infos->first());
```
