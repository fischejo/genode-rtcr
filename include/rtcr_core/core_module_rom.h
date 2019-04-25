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
#include <rtcr_core/core_module_base.h>
#include <rtcr_core/rom/rom_session.h>


namespace Rtcr {
    class Core_module_rom;
}

using namespace Rtcr;

class Rtcr::Core_module_rom: public virtual Core_module_base
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

protected:
    Rom_root *_rom_root;
    Genode::Local_service *_rom_service;
    Genode::Rom_connection *_rom_connection;


public:    
    /* implement virtual methods of Core_module_base */
    Rom_root &rom_root() {
	return *_rom_root;
    }
  
    Genode::Local_service &rom_service() {
	return *_rom_service;
    }

    Genode::Rom_connection &rom_connection() {
	return *_rom_connection;
    }
  

    Core_module_rom(Genode::Env &env,
		    Genode::Allocator &md_alloc,
		    Genode::Entrypoint &ep);

    void _initialize_rom_session(const char* label, bool &bootstrap);
    ~Core_module_rom();
  
};


#endif /* _RTCR_CORE_MODULE_ROM_H_ */
