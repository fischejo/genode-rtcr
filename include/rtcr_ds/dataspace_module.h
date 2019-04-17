/*
 * \brief  Dataspace Module
 * \author Johannes Fischer
 * \date   2019-04-13
 */

#ifndef _RTCR_DATASPACE_MODULE_H_
#define _RTCR_DATASPACE_MODULE_H_

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

#include <rtcr/dataspace_translation_info.h>

namespace Rtcr {
    class Dataspace_module;
    class Dataspace_module_factory;
}

using namespace Rtcr;


class Rtcr::Dataspace_module : public virtual Module
{
private:  
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

    Genode::List<Dataspace_translation_info> _dataspace_translations;

    void _checkpoint_dataspace(Target_state &state,
		      Genode::Dataspace_capability dst_ds_cap,
		      Genode::Dataspace_capability src_ds_cap,
		      Genode::addr_t dst_offset,
		      Genode::size_t size);
  
public:

  Dataspace_module(Genode::Env &env,
		   Genode::Allocator &md_alloc,
		   Genode::Entrypoint &ep);

  ~Dataspace_module();

    void register_dataspace(Genode::Ram_dataspace_capability ckpt_ds_cap,
			    Genode::Dataspace_capability resto_ds_cap,
			    Genode::size_t size);

    void checkpoint(Target_state &state);
    void restore(Target_state &state);
    Genode::Service *resolve_session_request(const char *service_name, const char *args);

    
  Module_name name()
  {
    return "dataspace";
  }
  
};



class Rtcr::Dataspace_module_factory : public Module_factory
{
public:
  Module* create(Genode::Env &env,
		 Genode::Allocator &md_alloc,
		 Genode::Entrypoint &ep,
		 const char* label,
		 bool &bootstrap,
		 Genode::Xml_node *config)  
  {
    return new (md_alloc) Dataspace_module(env, md_alloc, ep);
  }
    
  Module_name name()
  {
    return "dataspace";
  }
  
};

#endif /* _RTCR_DATASPACE_MODULE_H_ */
