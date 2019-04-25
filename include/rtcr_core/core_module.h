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
#include <rtcr_ds/dataspace_module.h>

namespace Rtcr {
    class Core_module;
    class Core_module_factory;
}

using namespace Rtcr;


class Rtcr::Core_module : public virtual Core_module_base,
			  public Core_module_pd,
			  public Core_module_cpu,
			  public Core_module_ram,
			  public Core_module_rm,
                          public Core_module_rom
{
private:
  Dataspace_module *_ds_module;
  Core_state &_state;


protected:
  Core_state &_initialize_state(Genode::Allocator &alloc);
  
public:  

  Core_module(Genode::Env &env,
	      Genode::Allocator &alloc,
	      Genode::Entrypoint &ep,
	      const char* label,
	      bool &bootstrap,
	      Genode::Xml_node *config);

  ~Core_module();

    void initialize(Genode::List<Module> &modules);
  
    Module_state *checkpoint();
    void restore(Module_state *state);

    Genode::Service *resolve_session_request(const char *service_name, const char *args);

  virtual Dataspace_module &ds_module()
  {
    return *_ds_module;
  }

  virtual Core_state &state()
  {
    return _state;
  }

  Module_name name(){
    return "core";
  }
};


// create a factory class for Cpu_session_handler
class Rtcr::Core_module_factory : public Module_factory
{
public:
  Module* create(Genode::Env &env,
		 Genode::Allocator &alloc,
		 Genode::Entrypoint &ep,
		 const char* label,
		 bool &bootstrap,
		 Genode::Xml_node *config)
  {   
    return new (alloc) Core_module(env, alloc, ep, label, bootstrap, config);
  }
    
  Module_name name()
  {
    return "core";
  }
  
};

#endif /* _RTCR_CORE_MODULE_H_ */
