/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_CORE_MODULE_ROM_H_
#define _RTCR_CORE_MODULE_ROM_H_

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
#include <rtcr_core/rom/rom_session.h>


namespace Rtcr {
    class Core_module_rom;
}

using namespace Rtcr;

class Rtcr::Core_module_rom: public Core_module_base
{
private:
    Rom_root &_rom_root;
    Genode::Local_service &_rom_service;
    Genode::Rom_connection &_rom_connection;
	
    /* implement virtual methods of Core_module_base */
    Rom_root & rom_root() {
	return _rom_root;
    }
  
    Genode::Local_service &rom_service() {
	return _rom_service;
    }

    Genode::Rom_connection &rom_connection() {
	return _rom_connection;
    }
  
public:
    Rom_session_handler(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &ep,
			const char* label,
			bool &bootstrap);
	
    ~Rom_session_handler();
  
};


#endif /* _RTCR_CORE_MODULE_ROM_H_ */
