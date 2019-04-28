/*
 * \brief  Log Module
 * \author Johannes Fischer
 * \date   2019-04-19
 */

#ifndef _RTCR_LOG_MODULE_H_
#define _RTCR_LOG_MODULE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>
#include <util/list.h>
#include <os/config.h>
#include <base/service.h>

/* Rtcr includes */
#include <rtcr/module.h>
#include <rtcr/module_state.h>
#include <rtcr/module_factory.h>
#include <rtcr/core_module_abstract.h>

/* Log module includes */
#include <rtcr_log/log_state.h>

namespace Rtcr {
	class Log_module;
	class Log_module_factory;
}

using namespace Rtcr;

/**
 * The Rtcr::Log_module provides a Log session and is able to checkpoint/restore
 * this session.
 */
class Rtcr::Log_module : public virtual Module
{
private:
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;
	Genode::Entrypoint &_ep;

protected:
	Log_root *_log_root;
	Genode::Local_service *_log_service;
	bool &_bootstrap;
	Core_module_abstract *_core_module;
	Log_state &_log_state;
  
	void _destroy_stored_log_session(Stored_log_session_info &stored_info);

	/**
	 * Create a Module_state object with the given allocator
	 */
	Log_state &_initialize_state(Genode::Allocator &alloc);
  
public:
  
	Log_module(Genode::Env &env,
		   Genode::Allocator &alloc,
		   Genode::Entrypoint &ep,
		   bool &bootstrap);

	~Log_module();

	/**
	 * Called by the Target_child when all modules are initialized.
	 *
	 * \param modules which were loaded. 
	 */
	void initialize(Genode::List<Module> &modules) override;

	/**
	 * Checkpoint LOG session
	 *
	 * \return the internal Log_state object which contains the checkpointed
	 *         state of the LOG session.
	 */
	Module_state *checkpoint() override;

	/**
	 * Restore this module to a state.
	 *
	 * Not yet implemented.
	 */    
	void restore(Module_state *state) override;

	/**
	 * If the child requests a LOG service, this module provides it.
	 */    
	Genode::Service *resolve_session_request(const char *service_name,
						 const char *args) override;
    
	Module_name name() override { return "log"; }  
};

/**
 * Factory class for creating the Rtcr::Log_module
 */
class Rtcr::Log_module_factory : public Module_factory
{
public:
	Module* create(Genode::Env &env,
		       Genode::Allocator &alloc,
		       Genode::Entrypoint &ep,
		       const char* label,
		       bool &bootstrap,
		       Genode::Xml_node *config) override
	{    
		return new (alloc) Log_module(env, alloc, ep,  bootstrap);
	}
    
	Module_name name() override { return "log"; }
};

#endif /* _RTCR_LOG_MODULE_H_ */
