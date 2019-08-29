/*
 * \brief  Session Handler Factory
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_MODULE_SET_FACTORY_H_
#define _RTCR_MODULE_SET_FACTORY_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <base/env.h>
#include <os/config.h>
#include <util/string.h>

/* Rtcr includes */
#include <rtcr/module.h>


using namespace Rtcr;

namespace Rtcr {
	class Module_factory;
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
	 * \param ep         Entrypoint
	 * \param label      Label of ?
	 * \param bootstrap  Bootstrap indicator
	 * \param config     XML configuration of the XML module node.
	 * \return A instance of a Module class
	 */
	virtual Module* create(Genode::Env &env,
						   Genode::Allocator &alloc,
						   Genode::Entrypoint &ep,
						   const char* label,
						   bool &bootstrap,
						   Genode::Xml_node *config) = 0;
    
	/**
	 * Name of your module which you want to register.
	 * 
	 * This should return exactly the same name as the `name()` of your module.
	 * The `Target_child` uses it in order to load the correct modules which are
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


#endif /* _RTCR_MODULE_SET_FACTORY_H_ */
