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
session. The most simple way is by inheriting from `Rtcr::Root_component`
root-class and overriding the `_create_session(..)` and `_destroy_session`
factory method.

```C++
Rtcr::Pd_cdma_session *_create_session(Child_info *info, const char *args) override
{
	Pd_cdma_session *pd_session = new (_alloc) Pd_cdma_session(_env, _alloc, _ep, args, info);
	info->pd_session = pd_session;
	return pd_session;
}

void _destroy_session(Child_info *info, Pd_cdma_session *session) override
{
	Genode::destroy(_alloc, session);
	info->pd_session = nullptr;
}
```

As your custom root-class is implemented, this has to be bundled with all other
root classes in a module-class.

## Custom Module Class

In general, the module class is just a wrapper around the `Init_module` which
holds all the logic for pausing, checkpointing, resuming the child
applications. Your module shall inherit from the `Init_module` and also
initializes all root objects in the constructor.

```C++
Cdma_module::Cdma_module(Genode::Env &env, Genode::Allocator &alloc)
	:
	Init_module(env, alloc),
	_ep(env, 16*1024, "resources ep"),
	_pd_factory(env, alloc, _ep, _childs_lock, _childs, _services),
	_cpu_factory(env, alloc, _ep, _childs_lock, _childs, _services),
	_log_factory(env, alloc, _ep, _childs_lock, _childs, _services),
	_timer_factory(env, alloc, _ep, _childs_lock, _childs, _services),
	_rom_factory(env, alloc, _ep, _childs_lock, _childs, _services),
	_rm_factory(env, alloc, _ep, _childs_lock, _childs, _services)
	{}
```

Now you are ready to test your module. If you want to initialize your module
based on the module name, as it is used in the `rtcr_app` application, it will
be necessary to implement a factory for your module with help of the
`Rtcr::Module_factory` class.

```C++
class Rtcr::Cdma_module_factory : public Module_factory
{
public:
	Init_module* create(Genode::Env &env, Genode::Allocator &alloc) override {
		return new (alloc) Cdma_module(env, alloc);
	}
    
	Module_name name() override { return "cdma"; }
};
```

In order to register the factory, initialize an instance as static object in
your `*.cc` file.

```C++
/* Create a static instance of the Init_module_factory. This registers the
 * module */
Rtcr::Cdma_module_factory _cdma_module_factory_instance;
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


