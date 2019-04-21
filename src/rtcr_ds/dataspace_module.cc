/*
 * \brief  Datapsace module
 * \description This module provides the dataspace handling
 * \author Johannes Fischer
 * \date   2019-04-13
 */

#include <rtcr_ds/dataspace_module.h>

using namespace Rtcr;

/* Create a static instance of the Dataspace_module_factory. This registers the module */
Rtcr::Dataspace_module_factory _dataspace_module_factory_instance;


Dataspace_module::Dataspace_module(Genode::Env &env,
				   Genode::Allocator &md_alloc,
				   Genode::Entrypoint &ep)

  :
  _env(env),
  _md_alloc(md_alloc),
  _ep(ep)
{

}


void Dataspace_module::checkpoint(Target_state &state)
{
#ifdef DEBUG
    Genode::log("\e[38;5;204m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
  Dataspace_translation_info *memory_info = _dataspace_translations.first();

#ifdef DEBUG
   Genode::log("Dataspaces to checkpoint:");  
    while(memory_info) {
	Genode::log(" ", *memory_info);
	memory_info = memory_info->next();
    }
#endif

  while(memory_info) {
    /* Dataspace is not managed */
    _checkpoint_dataspace(state,
			  memory_info->ckpt_ds_cap,
			  memory_info->resto_ds_cap,
			  0,
			  memory_info->size);

    memory_info = memory_info->next();

  }

}


void Dataspace_module::_checkpoint_dataspace(Target_state &state,
					     Genode::Dataspace_capability dst_ds_cap,
					     Genode::Dataspace_capability src_ds_cap,
					     Genode::addr_t dst_offset,
					     Genode::size_t size)
{
#ifdef DEBUG
    Genode::log("\e[38;5;204m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    
  char *dst_addr_start = state._env.rm().attach(dst_ds_cap);
  char *src_addr_start = state._env.rm().attach(src_ds_cap);

  Genode::memcpy(dst_addr_start + dst_offset, src_addr_start, size);

  state._env.rm().detach(src_addr_start);
  state._env.rm().detach(dst_addr_start);
}


void Dataspace_module::register_dataspace(Genode::Ram_dataspace_capability ckpt_ds_cap,
					  Genode::Dataspace_capability resto_ds_cap,
					  Genode::size_t size)
{
#ifdef DEBUG
    Genode::log("\e[38;5;204m", __PRETTY_FUNCTION__, "\033[0m");
#endif

  Dataspace_translation_info *trans_info = _dataspace_translations.first();
  if(trans_info)
    trans_info = trans_info->find_by_resto_badge(resto_ds_cap.local_name());
  if(!trans_info) {
    trans_info = new (_md_alloc) Dataspace_translation_info(ckpt_ds_cap,
							    resto_ds_cap,
							    size);
    _dataspace_translations.insert(trans_info);
  }  
}

void Dataspace_module::restore(Target_state &state)
{

}


Genode::Service *Dataspace_module::resolve_session_request(const char *service_name,
							   const char *args)
{
  return 0;
}


