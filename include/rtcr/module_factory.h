/*
 * \brief  Module Factory
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_MODULE_FACTORY_H_
#define _RTCR_MODULE_FACTORY_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <base/env.h>
#include <util/string.h>

/* Rtcr includes */
#include <rtcr/init_module.h>


using namespace Rtcr;

namespace Rtcr {
	class Module_factory;
	template <typename> struct Module_factory_builder;
}


/**
 * Factory and registry class for modules
 *
 * This class implements the registry pattern in order to register linked
 * modules during runtime. With the `create` method, a factory for initializing
 * new modules is provided.
 * 
 * In order to register a module:
 *
 * 1. Create a customized module factory class by inheriting
 *    `Rtcr::Module_factory` and implement the virtual methods.
 *
 * 2. Create a static object of your customized module factory.
 *
 * During execution, this static object registers itself and provides a factory
 * method for your `Module` class.
 */
class Rtcr::Module_factory
{
private:
	/**
	 * Due to some unknown bug, Genode::List can not be initialized
	 * statically. Therefore a simple list is implemented here. 
	 */
	static Module_factory* _head;
	Module_factory* _next;
public:
	/**
	 * Factory Method which creates a instance of a `Module` class.
	 *
	 * \param env        Environment
	 * \param alloc      Heap Allocator
	 * \return A instance of a Module class
	 */
	virtual Init_module* create(Genode::Env &env,
	                            Genode::Allocator &alloc) = 0;
    
	/**
	 * Name of your module which you want to register.
	 * 
	 * This should return exactly the same name as the `name()` of your module.
	 * The `Child` uses it in order to load the correct modules which are
	 * defined in the XML configuration.
	 *
	 * \return Name of your module
	 */
	virtual Module_name name() = 0;
  
	Module_factory();
	~Module_factory();
    
	/* \brief Methods which implement the list operations */
	static void print();
	static Module_factory* get(const Module_name name);
	
	static Module_factory* first();
	Module_factory* next();
};


/**
 * Builder template for creating a Module_factory
 */
template <typename MODULE>
struct Rtcr::Module_factory_builder : Module_factory
{
	Module_factory_builder() : Module_factory() {};

	virtual Init_module* create(Genode::Env &env,
	                            Genode::Allocator &alloc) override
	{
		return new(alloc) MODULE(env, alloc);
	}

	virtual Module_name name() { return MODULE::name(); }
};


#endif /* _RTCR_MODULE_FACTORY_H_ */
