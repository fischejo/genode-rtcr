/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_CORE_MODULE_RM_H_
#define _RTCR_CORE_MODULE_RM_H_

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
#include <rtcr_core/rm/rm_session.h>


namespace Rtcr {
    class Core_module_rm;
}

using namespace Rtcr;

class Rtcr::Core_module_rm: public Core_module_base
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

    Rm_root &_rm_root;
    Genode::Local_service &_rm_service;

  /* implement virtual methods of Core_module_base */
  Rm_root & rm_root() {
    return _rm_root;
  }
  
  Genode::Local_service &rm_service() {
    return _rm_service;
  }

    Genode::List<Ref_badge_info> _region_maps;
    
public:
    Rm_session_handler(Genode::Env &env,
		       Genode::Allocator &md_alloc,
		       Genode::Entrypoint &ep,
		       const char* label,
		       bool &bootstrap);
	
    ~Rm_session_handler();


  
};


#endif /* _RTCR_CORE_MODULE_RM_H_ */
