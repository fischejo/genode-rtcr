/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_CPU_SESSION_HANDLER_H_
#define _RTCR_CPU_SESSION_HANDLER_H_

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
#include <rtcr/session_handler.h>
#include <rtcr/session_handler_factory.h>

#include <rtcr_cpu/cpu_session.h>
#include <rtcr_pd/pd_session.h>
#include <rtcr_pd/pd_session_handler.h>



namespace Rtcr {
    class Cpu_session_handler;
    class Cpu_session_handler_factory;
}

using namespace Rtcr;

class Rtcr::Cpu_session_handler: public Session_handler
{
private:

    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;	

    
    Cpu_root &_root;
    Genode::Local_service &_service;
    Cpu_session_component &_session;

    Pd_session_handler &_pd_handler;
    

    void _prepare_cpu_sessions(Target_state &state, Genode::List<Cpu_session_component> &child_infos);
    void _destroy_stored_cpu_session(Target_state &state, Stored_cpu_session_info &stored_info);

    void _prepare_cpu_threads(Target_state &state, , Genode::List<Cpu_thread_component> &child_infos);
    void _destroy_stored_cpu_thread(Target_state &state, Stored_cpu_thread_info &stored_info);

    Cpu_session_component &_find_session(const char *label, Cpu_root &cpu_root);    

    /**
     * Pause all child's threads
     */
    void _pause();
    /**
     * Resume all child's threads
     */
    void _resume();

    

    
public:
    Cpu_session_handler(Genode::Env &env,
			Genode::Allocator &md_alloc,
			Genode::Entrypoint &ep,
			Pd_session_handler &pd_handler,
			const char* label,
			bool &bootstrap);

    ~Cpu_session_handler();

    
    void prepare_checkpoint(Target_state &state);
    void checkpoint(Target_state &state);
    void post_checkpoint(Target_state &state);

    void session_restore();


    /**
     * Getter for `Cpu_root`
     */
    Cpu_root &root();

    /**
     * Getter for `Local_service` of CPU session
     */    
    Genode::Local_service &service();

    /**
     * Getter for `Cpu_session_component`
     */        
    Cpu_session_component &session();

    
    char const* name() {
	return "CPU";
    }









    
};


// create a factory class for Cpu_session_handler
class Rtcr::Cpu_session_handler_factory : public Session_handler_factory
{
public:
    Session_handler* create(Genode::Env &env,
			    Genode::Allocator &md_alloc,
			    Genode::Entrypoint &ep,
			    Genode::List<Session_handler> *_session_handlers,			  
			    const char* label,
			    bool &bootstrap)
	{
    
	    Session_handler *session_handler = _session_handlers->first();
	    Pd_session_handler *pd_session_handler = NULL;
	    while (!pd_session_handler && session_handler)
		{
		    // try to cast to Pd_session_handler
		    pd_session_handler = dynamic_cast<Pd_session_handler*>(session_handler);
		    session_handler = session_handler->next();
		}

	    if(!pd_session_handler)
		Genode::error("No PD session handler found! ");
    
	    return new (md_alloc) Cpu_session_handler(env,
						      md_alloc,
						      ep,
						      pd_handler,
						      label,
						      bootstrap);
	}
    
    char const* name()
	{
	    return "CPU";
	}
  
};

#endif /* _RTCR_CPU_SESSION_HANDLER_H_ */
