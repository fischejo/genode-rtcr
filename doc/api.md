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
Target_child sheep (env, heap, "sheep_counter", module);
sheep.start();

```

Multiple child are supported.


# Checkpointing

```C++
Child_info *info = module.child_info("sheep_counter");
module.pause(info);
module.checkpoint(info);
module.resume(info);
```

The last checkpointed state can be examined:

```C++
Genode::log(child_info);
```

# Serialization

The `Serializer` class compress the last checkpoint of `sheep` and provides it
as dataspace `ds_cap`. The actual size of data in the dataspace is stored in the
variable `size`. 

```C++
Serializer s(env, heap);

Genode::size_t size;
Genode::Ram_dataspace_capability ds_cap = s.serialize(info, &size);
```

