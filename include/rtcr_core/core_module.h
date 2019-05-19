/*
 * \brief  Core Module
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#ifndef _RTCR_CORE_MODULE_H_
#define _RTCR_CORE_MODULE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>
#include <os/config.h>
#include <base/service.h>

/* Local includes */
#include <rtcr/module.h>
#include <rtcr/module_factory.h>
#include <rtcr/module_state.h>
#include <rtcr_core/core_module_base.h>
#include <rtcr_core/core_module_cpu.h>
#include <rtcr_core/core_module_ram.h>
#include <rtcr_core/core_module_pd.h>
#include <rtcr_core/core_module_rm.h>
#include <rtcr_core/core_module_rom.h>
#include <rtcr_core/core_module_ds.h>

namespace Rtcr {
	class Core_module;
	class Core_module_factory;
}

using namespace Rtcr;


/**
 * The class Rtcr::Core_module provides the a simple and minimal implementation
 * of the Rtcr::Core_module_abstract class. The implementation is split up in
 * several class. More about that in class Core_module_base.
 */
class Rtcr::Core_module : public virtual Core_module_base,
			  public Core_module_pd,
			  public Core_module_cpu,
			  public Core_module_ram,
			  public Core_module_rm,
                          public Core_module_rom,
                          public Core_module_ds
{
private:

	Core_state &_state;

protected:

	/**
	 * Create a Module_state object with the given allocator
	 */
	Core_state &_initialize_state(Genode::Allocator &alloc);
    
public:  

	Core_module(Genode::Env &env,
		    Genode::Allocator &alloc,
		    Genode::Entrypoint &ep,
		    const char* label,
		    bool &bootstrap,
		    Genode::Xml_node *config);

	~Core_module();

	/**
	 * Checkpoint PD,RAM,ROM,RM,CPU sessions
	 *
	 * \return the internal Core_state object which contains the checkpointed
	 *         state of the sessions.
	 */
	Module_state *checkpoint() override;

	/**
	 * Restore this module to a state.
	 *
	 * Not yet implemented.
	 */    
	void restore(Module_state *state) override;

	/**
	 * If the child requests a service of PD,RM,RAM,ROM or CPU, this module provides it.
	 */    
	Genode::Service *resolve_session_request(const char *service_name, const char *args) override;

	/**
	 * Get state of this module
	 *
	 * \return Core_state object 
	 */    
	virtual Core_state &state() override { return _state; }

	Module_name name() override { return "core"; }
};

/**
 * Factory class for creating the Rtcr::Core_module
 */
class Rtcr::Core_module_factory : public Module_factory
{
public:
	Module* create(Genode::Env &env,
		       Genode::Allocator &alloc,
		       Genode::Entrypoint &ep,
		       const char* label,
		       bool &bootstrap,
		       Genode::Xml_node *config) override
	{   
		return new (alloc) Core_module(env, alloc, ep, label, bootstrap, config);
	}
    
	Module_name name() override { return "core"; }
  
};

#endif /* _RTCR_CORE_MODULE_H_ */
