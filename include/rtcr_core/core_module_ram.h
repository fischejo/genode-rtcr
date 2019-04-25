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
#include <rtcr_core/core_module_base.h>
#include <rtcr/module_state.h>

#include <rtcr_core/ram/ram_session.h>
#include <rtcr/dataspace_translation_info.h>
#include <rtcr/ref_badge_info.h>


namespace Rtcr {
    class Core_module_ram;
}

using namespace Rtcr;


class Rtcr::Core_module_ram: public virtual Core_module_base
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_alloc;
    Genode::Entrypoint &_ep;	

protected:  
    Ram_root *_ram_root;
    Genode::Local_service *_ram_service;
    Ram_session_component *_ram_session;

    void _initialize_ram_session(const char* label, bool &bootstrap);
    Ram_session_component *_find_ram_session(const char *label, Ram_root &ram_root);
  
    // level: 1.1
    void _prepare_ram_dataspaces(Genode::List<Stored_ram_dataspace_info> &stored_infos,
				 Genode::List<Ram_dataspace_info> &child_infos);
    // level 1.1.1
    Stored_ram_dataspace_info &_create_stored_ram_dataspace(Ram_dataspace_info &child_info);
    
    // level: 1.2
    void _destroy_stored_ram_session(Stored_ram_session_info &stored_info);

    // level: 1.2.1
    void _destroy_stored_ram_dataspace(Stored_ram_dataspace_info &stored_info);    



    // level: 1    
    void _checkpoint();

    

public:
    
    virtual Genode::Local_service &ram_service() {
	return *_ram_service;
    }
    virtual Genode::Rpc_object<Genode::Ram_session> &ram_session() {
	return *_ram_session;
    }
  

    Core_module_ram(Genode::Env &env,
		    Genode::Allocator &alloc,
		    Genode::Entrypoint &ep);

	
    ~Core_module_ram();

    /* implement virtual methods of Core_module_base */
    Ram_root &ram_root() {
	return *_ram_root;
    }

    
};


#endif /* _RTCR_CORE_MODULE_RAM_H_ */
