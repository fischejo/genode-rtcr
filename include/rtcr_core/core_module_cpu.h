/*
 * \brief  CPU implementation for core module
 * \author Johannes Fischer
 * \date   2019-04-04
 */

#ifndef _RTCR_CORE_MODULE_CPU_H_
#define _RTCR_CORE_MODULE_CPU_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <cpu_session/cpu_session.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>
#include <util/list.h>

/* Local includes */
#include <rtcr/module_state.h>
#include <rtcr_core/core_state.h>
#include <rtcr_core/cpu/cpu_session.h>
#include <rtcr_core/core_module_base.h>
#include <rtcr_core/core_module_pd.h>

namespace Rtcr {
    class Core_module_cpu;
}



using namespace Rtcr;

class Rtcr::Core_module_cpu : public virtual Core_module_base
{

 private:
    Genode::Env        &_env;
    Genode::Allocator  &_alloc;
    Genode::Entrypoint &_ep;	

 protected:  
    Cpu_root *_cpu_root = nullptr;
    Genode::Local_service *_cpu_service = nullptr;
    Cpu_session_component *_cpu_session = nullptr;

    Cpu_session_component *_find_cpu_session(const char *label, Cpu_root &cpu_root);      
    void _initialize_cpu_session(const char* label, bool &bootstrap);


    // level 1.1
    void _prepare_cpu_threads(Genode::List<Stored_cpu_thread_info> &stored_infos,			      
			      Genode::List<Cpu_thread_component> &child_infos);

    
    // level 1.2
    void _destroy_stored_cpu_session(Stored_cpu_session_info &stored_info);

    // level 1.2.1
    void _destroy_stored_cpu_thread(Stored_cpu_thread_info &stored_info);



    // level 1
    void _checkpoint();
    
  

public:
    /**n
     * Pause all child's threads
     */
    virtual void pause();
    /**
     * Resume all child's threads
     */
    virtual void resume();  

    
    virtual Genode::Local_service &cpu_service() {
	return *_cpu_service;
    };
  
    virtual Genode::Rpc_object<Genode::Cpu_session> &cpu_session() {
	return *_cpu_session;
    };

    
    Core_module_cpu(Genode::Env &env,
		    Genode::Allocator &alloc,
		    Genode::Entrypoint &ep);

    ~Core_module_cpu();

    /* implement virtual methods of Core_module_base */
    Cpu_root &cpu_root() {
	return *_cpu_root;
    };
    
};


#endif /* _RTCR_CORE_MODULE_CPU_H_ */
