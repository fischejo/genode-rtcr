/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_RM_SESSION_HANDLER_H_
#define _RTCR_RM_SESSION_HANDLER_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>


/* Global includes */
#include <rtcr/session_handler.h>
#include <rtcr/session_handler_factory.h>

/* Local includes */
#include <rtcr_rm/rm_session.h>

namespace Rtcr {
    class Rm_session_handler;
    class Rm_session_handler_factory;  
}

using namespace Rtcr;

class Rtcr::Rm_session_handler: public Session_handler
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

    Rm_root &_root;
    Genode::Local_service &_service;
//    Rm_session_component &_session;

    Genode::List<Ref_badge_info> _region_maps;
    
public:
    Rm_session_handler(Genode::Env &env,
		       Genode::Allocator &md_alloc,
		       Genode::Entrypoint &ep,
		       const char* label,
		       bool &bootstrap);
	
    ~Rm_session_handler();


    void prepare_checkpoint(Target_state &state);
    void checkpoint(Target_state &state);
    void post_checkpoint(Target_state &state);

    void session_restore();

    /**
     * Getter for `Cpu_root`
     */
    Rm_root &root();

    /**
     * Getter for `Local_service` of CPU session
     */    
    Genode::Local_service &service();

    /**
     * Getter for `Cpu_session_component`
     */        
    Rm_session_component &session();



};

// create a factory class for Cpu_session_handler
class Rtcr::Rm_session_handler_factory : public Session_handler_factory
{
public:
    Session_handler* create(Genode::Env &env,
			    Genode::Allocator &md_alloc,
			    Genode::Entrypoint &ep,
			    Genode::List<Session_handler> *_session_handlers,
			    const char* label,
			    bool &bootstrap)
	{
	    return new (md_alloc) Rm_session_handler(env, md_alloc, ep, label, bootstrap);
	}
    
    char const* name()
	{
	    return "RM";
	}
  
};



#endif /* _RTCR_RM_SESSION_HANDLER_H_ */
