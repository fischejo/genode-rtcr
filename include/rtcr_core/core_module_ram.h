/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_CORE_MODULE_RAM_H_
#define _RTCR_CORE_MODULE_RAM_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>


/* Local includes */
#include <rtcr_core/core_module.h>
#include <rtcr/target_state.h>

#include <rtcr_core/ram/ram_session.h>
#include <rtcr/dataspace_translation_info.h>
#include <rtcr/ref_badge_info.h>
#include <rtcr_core/ram/simplified_managed_dataspace_info.h>


namespace Rtcr {
    class Core_module_ram;
}

using namespace Rtcr;


class Rtcr::Core_module_ram: public Core_module_base
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;	

    Ram_root &_ram_root;
    Genode::Local_service &_ram_service;
    Ram_session_component   &_ram_session;

    /**
     * Mapping to find a copy dataspace for a given original dataspace badge
     */
    Genode::List<Dataspace_translation_info> _dataspace_translations;
    Genode::List<Simplified_managed_dataspace_info> _managed_dataspaces;


    // level: 1    
    void _checkpoint(Target_state &state);

    // level: 1.1
    void _prepare_ram_dataspaces(Target_state &state,
				 Genode::List<Stored_ram_dataspace_info> &stored_infos,
				 Genode::List<Ram_dataspace_info> &child_infos);
    // level 1.1.1
    Stored_ram_dataspace_info &_create_stored_ram_dataspace(Target_state &state,
							    Ram_dataspace_info &child_info);
    
    // level: 1.2
    void _destroy_stored_ram_session(Target_state & state,
				     Stored_ram_session_info &stored_info);

    // level: 1.2.1
    void _destroy_stored_ram_dataspace(Target_state &state,
				       Stored_ram_dataspace_info &stored_info);    


    
    // level 2
    void _checkpoint_temp_wrapper(Target_state &state);

    // level 2.1
    VOID _create_managed_dataspace_list();

    // level 2.2
    void _detach_designated_dataspaces();

    // level: 2.3
    void _checkpoint_dataspaces(Target_state &state);

    // level 2.3.1
    void _checkpoint_dataspace_content(Target_state &state,
				       Genode::Dataspace_capability dst_ds_cap,
				       Genode::Dataspace_capability src_ds_cap,
				       Genode::addr_t dst_offset,
				       Genode::size_t size);

    
    /* implement virtual methods of Core_module_base */
    Ram_root & ram_root() {
	return _ram_root;
    }
  
    Genode::Local_service &ram_service() {
	return _ram_service;
    }
    Ram_session_component &ram_session() {
	return _ram_session;
    }
  
public:
    Ram_session_handler(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &ep,
			const char* label,
			Genode::size_t granularity,
			bool &bootstrap);
	
    ~Ram_session_handler();

  	
};


#endif /* _RTCR_CORE_MODULE_RAM_H_ */
