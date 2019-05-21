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

/* Rtcr includes */
//#include <rtcr/dataspace_translation_info.h>
#include <rtcr/module_state.h>
#include <rtcr/ref_badge_info.h>

/* Core module includes */
#include <rtcr_core/core_module_base.h>
#include <rtcr_core/ram/ram_session.h>

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

	virtual void _prepare_ram_dataspaces(Genode::List<Stored_ram_dataspace_info> &stored_infos,
				     Genode::List<Ram_dataspace_info> &child_infos);

	virtual Stored_ram_dataspace_info &_create_stored_ram_dataspace(Ram_dataspace_info &child_info);
    
	void _destroy_stored_ram_session(Stored_ram_session_info &stored_info);

	void _destroy_stored_ram_dataspace(Stored_ram_dataspace_info &stored_info);    

	void _checkpoint();

public:
	Core_module_ram(Genode::Env &env,
			Genode::Allocator &alloc,
			Genode::Entrypoint &ep);
	
	~Core_module_ram();
    
	virtual Genode::Local_service &ram_service() override
	{
		return *_ram_service;
	}

	virtual Genode::Rpc_object<Genode::Ram_session> &ram_session() override
	{
		return *_ram_session;
	}

	Ram_root &ram_root() override
	{
		return *_ram_root;
	}
};


#endif /* _RTCR_CORE_MODULE_RAM_H_ */
