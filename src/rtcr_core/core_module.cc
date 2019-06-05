/*
 * \brief  Core module
 * \description This module provides a minimal implementation for a sucessful checkpoint/restore.
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#include <rtcr_core/core_module.h>

using namespace Rtcr;

/* Create a static instance of the Core_module_factory. This registers the module */
Rtcr::Core_module_factory _core_module_factory_instance;


Core_module::Core_module(Genode::Env &env,
			 Genode::Allocator &alloc,
			 Genode::Entrypoint &ep,
			 const char* label,
			 bool &bootstrap,
			 Genode::Xml_node *config)
	:
	_state(_initialize_state(alloc)),
	Core_module_pd(env, alloc, ep),
	Core_module_cpu(env, alloc, ep ), /* depends on Core_module_pd::Core_module_pd() */
	Core_module_rm(env, alloc, ep), /* depends on Core_module_pd::Core_module_pd() */
	Core_module_ram(env, alloc, ep), /* depends on Core_module_pd::Core_module_pd() */
	Core_module_rom(env, alloc, ep),
	Core_module_ds(env, alloc, ep)
{
	_initialize_pd_session(label, bootstrap);
	_initialize_cpu_session(label, bootstrap);
	_initialize_rm_session(label, bootstrap);
	_initialize_ram_session(label, bootstrap);
	_initialize_rom_session(label, bootstrap);
	_transfer_quota(config);
}


Core_state &Core_module::_initialize_state(Genode::Allocator &alloc)
{
	return *new(alloc) Core_state(alloc);
}


Module_state *Core_module::checkpoint()
{
	/* initialize `kcap_mappings` variable. This step depends on Core_module_ram::Core_module_ram() */
	Core_module_pd::_create_kcap_mappings();

	/* initialize `region_map` variable. This step depends on Core_module_pd::Core_module_pd() */
	Core_module_rm::_create_region_map_dataspaces_list();

	/* Checkpointing */
	Core_module_pd::_checkpoint();  /* depends on Core_module_pd::_create_kcap_mappings */
	Core_module_cpu::_checkpoint();  /* depends on Core_module_pd::_create_kcap_mappings */
	Core_module_rm::_checkpoint();  /* depends on Core_module_pd::_create_kcap_mappings */

	/* depends on Core_module_pd::_create_kcap_mappings */
	/* depends on Core_module_rm::_create_region_map_dataspace_list */    
	Core_module_ram::_checkpoint();

	Core_module_ds::_checkpoint();
	
	return &_state;
}


void Core_module::restore(Module_state *state)
{

}


Genode::Service *Core_module::resolve_session_request(const char *service_name,
						      const char *args)
{

	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
  
	if(!Genode::strcmp(service_name, "PD")) {
		return &pd_service();
	} else if(!Genode::strcmp(service_name, "CPU")) {
		return &cpu_service();
	} else if(!Genode::strcmp(service_name, "RAM")) {
		return &ram_service();
	} else if(!Genode::strcmp(service_name, "RM")) {
		return &rm_service();	
	} else if(!Genode::strcmp(service_name, "ROM")) {
		return &rom_service();	
	} else {
		Genode::Service *service = 0;	
		return service;
	}
}
