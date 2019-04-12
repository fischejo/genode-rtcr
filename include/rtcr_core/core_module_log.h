/*
 * \brief  Log Session Handler
 * \author Johannes Fischer
 * \date   2019-04-12
 */

#ifndef _RTCR_CORE_MODULE_LOG_H_
#define _RTCR_CORE_MODULE_LOG_H_

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
#include <rtcr_core/log/log_session.h>
#include <rtcr/target_state.h>

namespace Rtcr {
    class Core_module_log;
}

using namespace Rtcr;

class Rtcr::Core_module_log: public virtual Core_module_base
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;
  
    Log_root *_log_root;
    Genode::Local_service *_log_service;    

    void _destroy_stored_log_session(Target_state &state,
				   Stored_log_session_info &stored_info);
 

public:    
    /* implement virtual methods of Core_module_base */
    Log_root &log_root() {
	return *_log_root;
    }
  
    Genode::Local_service &log_service() {
	return *_log_service;
    }

    Core_module_log(Genode::Env &env,
		    Genode::Allocator &md_alloc,
		    Genode::Entrypoint &ep);

    void _init(const char* label, bool &bootstrap);
    void _checkpoint(Target_state &state);  
  
    ~Core_module_log();


};


#endif /* _RTCR_CORE_MODULE_LOG_H_ */
