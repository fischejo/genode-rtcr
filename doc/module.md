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
session. The most simple way is by using the `Rtcr::Root_component` template. It
takes care of providing a valid `Child_info` objects to your session, creating a
`Genode::Service` object and announcing your custom session to the parent
application.


## Custom Module Class

In general, the module class is just a wrapper around the `Init_module` which
holds all the logic for pausing, checkpointing, resuming the child
applications. Your module shall inherit from the `Init_module` and also
initializes a root component for each session.

```C++
class Rtcr::Base_module : public Init_module
{
	Genode::Entrypoint _ep;    
	Root_component<Pd_session> _pd_root;
	Root_component<Cpu_session> _cpu_root;
	Root_component<Log_session> _log_root;
	Root_component<Timer_session> _timer_root;
	Root_component<Rom_session> _rom_root;
	Root_component<Rm_session> _rm_root;
	...
}
```

Now you are ready to test your module. If you want to initialize your module
based on the module name, it will be necessary to implement a factory for your
module with help of the `Rtcr::Module_factory` class.

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


