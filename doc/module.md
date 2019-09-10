# Modules

The Rtcr is modularised with different concepts:
1. Each module can be implemented in a new Git **Repository**. It will be
   installed to `genode/repos`
2. Each module is bundled to a static library, which is linked to the minimal
   `rtcr` during compile time. This allows to bundle a Rtcr with minimal size.
3. Each module is represented by a class which inherits from
   `Init_module`. This class is the entrypoint for adjusting all functionalities
   of the minimal `rtcr`.
   
For example: `rtcr_cdma`

This module implements a hardware-based acceleration of the memory copying. You
can find it in Git repository `rtcr_cdma`. It provides the library makefile
`rtcr_cdma` which is in `rtrc_cdma/lib/mk/rtcr_cdma.mk`. The module class is in
`rtcr_cdma/include/rtcr_cdma/cdma_module.h`.


# How to Create

Let's assume, you want to implement your own module. Following steps are
necessary. Before you start, compare `rtcr` and `rtcr_cdma` and understand how
C++ virtual methods are used to override methods of `rtcr`.

1. Create a Repository
   1. Create a Git Repository and clone it to `genode/repos/rtcr_X`
   2. Add the Repository to the `etc/build.conf` of your build directory
2. Create a Library
   1. Create `lib/mk/rtcr_X.mk` makefile. This defines a static library. 
3. Create a Module Class
   1. Implement `include/rtcr_X/X_module.h`
   2. Implement `src/rtcr_X/X_module.cc`


## Custom Session Class

The checkpoint logic is implemented in each session. If you want to change this
logic, you will inherit from a session which is provided by the minimal `rtcr`
and override parent methods. Finally you should have all your custom sessions and need
to initialize these. This is done by root classes.

## Custom Root Class

A root class creates a session whenever a child application requests it. For
this reason, it is necessary to also provide a custom root-class for your custom
session. The most simple way is by inheriting from an existing root-class and
overriding the `_create_ram_session(..)` factory method.

```C++
Ram_session *Ram_cdma_root::_create_ram_session(Child_info *info, const char *args)
{
	return new (md_alloc()) Ram_cdma_session(_env, _md_alloc, args, info);
}
```

As your custom root-class is implemented, this has to be bundled with all other
root classes in a module-class.

## Custom Module Class

In general, the module class is just a wrapper around the `Init_module` which
holds all the logic for pausing, checkpointing, resuming the children
applications. The `Init_module` also provides factory methods `init(...)` which
create all root objects. As you are interested in the initialization
of your custom root-class, these methods should be used in the constructor
of your module-class.

```C++
Cdma_module::Cdma_module(Genode::Env &env, Genode::Allocator &alloc)
	:
	Init_module(env, alloc)
{
	init( new(alloc) Ram_cdma_root(env, alloc, _ep, _childs_lock, _childs));
	...
}

```

## Configuration

Your session-class, root-class or module-class might depend on a
configuration. This is generally provided by the XML-configuration file attached
during packing of the image file. In order to keep the complexity low, I suggest
to not use nested XML nodes and hand define the configuration as simple as
possible. For example the RAM session of the module `rtcr_inc` depends on a
granularity value. This can be configured as follows:

```XML
<config>
	<granularity>4</granularity>
</config>
```


