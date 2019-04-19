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
#include <rtcr/module_factory.h>
#include <rtcr/target_state.h>
#include <rtcr/core_module_abstract.h>


namespace Rtcr {
    class Log_module;
    class Log_module_factory;
}

using namespace Rtcr;


class Rtcr::Log_module : public virtual Module
{
private:  
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

    Log_root *_log_root;
    Genode::Local_service *_log_service;
    bool &_bootstrap;
    Core_module_abstract *_core_module;
  
    void _destroy_stored_log_session(Target_state &state,
				   Stored_log_session_info &stored_info);

  
public:
  
  Log_module(Genode::Env &env,
	      Genode::Allocator &md_alloc,
	      Genode::Entrypoint &ep,
	      bool &bootstrap);

  ~Log_module();
    void initialize(Genode::List<Module> &modules);
    void checkpoint(Target_state &state);
    void restore(Target_state &state);
    Genode::Service *resolve_session_request(const char *service_name, const char *args);

  Module_name name(){
    return "log";
  }  
};


class Rtcr::Log_module_factory : public Module_factory
{
public:
  Module* create(Genode::Env &env,
		 Genode::Allocator &md_alloc,
		 Genode::Entrypoint &ep,
		 const char* label,
		 bool &bootstrap,
		 Genode::Xml_node *config)
  {    
    return new (md_alloc) Log_module(env, md_alloc, ep,  bootstrap);
  }
    
  Module_name name()
  {
    return "log";
  }
  
};

#endif /* _RTCR_LOG_MODULE_H_ */
