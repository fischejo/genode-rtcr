# Initialization of a Module

A module wraps the Root Component creation for all sessions. These sessions
finally intercept all IPC and service requests. The `base` module implements the
default checkpoint/restore. If the checkpoint/restore mechanism is extended,
another module will be implemented. The user has to choose one module in the
beginning.

```C++
Base_module module(env, heap);
```

It is possible to configure a module name in the XML config and use the factory
to initialize the module.

```C++
```



# Child Creation

```C++
Child sheep (env, heap, "sheep_counter", module);
sheep.start();

```

Multiple child are supported.


# Checkpointing
All childs are paused, checkpointed and resumed in parallel.

```C++
Child_info *info = module.child_info("sheep_counter");
module.pause();
module.checkpoint();
module.resume();
```

The last checkpointed state can be examined:

```C++
Genode::log(module.child_info("sheep_counter));
```

# Serialization

The `Serializer` class compress the last checkpoint of `sheep` and provides it
as dataspace `ds_cap`. The actual size of data in the dataspace is stored in the
variable `size`. 

```C++
Serializer s(env, heap);

Genode::size_t size;
Genode::Ram_dataspace_capability ds_cap = s.serialize(module.child_info(), &size);
```

