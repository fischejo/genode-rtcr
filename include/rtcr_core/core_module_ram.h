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
#include <rtcr_core/ram/ram_session.h>


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
