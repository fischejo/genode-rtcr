/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_PD_SESSION_HANDLER_H_
#define _RTCR_PD_SESSION_HANDLER_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>
#include <util/list.h

/* Global includes */
#include <rtcr/session_handler.h>
#include <rtcr/session_handler_factory.h>

#include <rtcr_pd/pd_session.h>

// new
#include <rtcr_pd/util/kcap_badge_info.h>
#include <rtcr_pd/util/ref_badge_info.h>

namespace Rtcr {
    class Pd_session_handler;
    class Pd_session_handler_factory;  
}

using namespace Rtcr;

class Rtcr::Pd_session_handler: public Session_handler
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

    Pd_root &_root;
    Genode::Local_service &_service;
    Pd_session_component   &_session;
    Ram_session_component *_ram_session;
    /**
     * Capability map in a condensed form
     * Refactored from `checkpointer.h`
     */
    Genode::List<Kcap_badge_info> _kcap_mappings;

    /**
     * Refactored from `target.child.h`. Previously `Target_child::Resources::_init_pd()`
     */
    Pd_session_component &_find_session(const char *label, Pd_root &pd_root);


    /**
     * \brief Destroys the _kcap_mappings list.
     */
    void _destroy_list(Genode::List<Kcap_badge_info> &list);    

    /**
     * \brief Prepares the capability map state_infos
     *
     * First the method fetches the capability map information from child's cap map structure in an
     * intercepted dataspace.
     *
     * Second it prepares the capability map state_infos.
     * For each badge-kcap tuple found in child's cap map the method checks whether a corresponding
     * list element in state_infos exists. If there is no list element, then it is created and marked.
     * Else it is just marked. After this, the old badge-kcap tuples, which where not marked, are deleted
     * from state_infos. Now an updated capability map is ready to used for the next steps to store the
     * kcap for each RPC object.
     */
    Genode::List<Kcap_badge_info> _create_kcap_mappings(Target_state &state);
    Genode::List<Ref_badge_info> _mark_and_attach_designated_dataspaces(Attached_region_info &ar_info);
    void _detach_and_unmark_designated_dataspaces(Genode::List<Ref_badge_info> &badge_infos, Attached_region_info &ar_info);

    
    void _prepare_pd_sessions(Target_state &state, Genode::List<Pd_session_component> &child_infos);
    void _destroy_stored_pd_session(Target_state &state, Stored_pd_session_info &stored_info);

    void _prepare_native_caps(Target_state &state, Genode::List<Native_capability_info> &child_infos);
    void _destroy_stored_native_cap(Target_state &state, Stored_native_capability_info &stored_info);

    void _prepare_signal_sources(Target_state &state, Genode::List<Signal_source_info> &child_infos);
    void _destroy_stored_signal_source(Target_state &state, Stored_signal_source_info &stored_info);

    void _prepare_signal_contexts(Target_state &state, Genode::List<Signal_context_info> &child_infos);
    void _destroy_stored_signal_context(Target_state &state, Stored_signal_context_info &stored_info);
    
    
public:
    Pd_session_handler(Genode::Env &env,
		       Genode::Allocator &md_alloc,
		       Genode::Entrypoint &ep,
		       const char* label,
		       bool &bootstrap,
		       Ram_session_component *ram_session)

    ~Pd_session_handler();

    /**
     * Getter for `Pd_root`
     */
    Pd_root &root();

    /**
     * Getter for `Local_service` of Pd session
     */    
    Genode::Local_service &service();

    /**
     * Getter for `Pd_session_component`
     */        
    Pd_session_component &session();

    /**
     * Getter for `kcap_mappings` list (Wer bruacht sie eigentlich?)
     */        
    Genode::List<Kcap_badge_info> &kcap_mappings();

    /**
     * \brief Return the kcap for a given badge from _capability_map_infos
     * Refactored from `checkpointer.h`
     * Return the kcap for a given badge. If there is no, return 0.
     */
    Genode::addr_t find_kcap_by_badge(Genode::uint16_t badge);

  
  
    void prepare_checkpoint(Target_state &state);
    void checkpoint(Target_state &state);
    void session_restore();

    char const* name() {
      return "PD";
    }
	
};



// create a factory class for Cpu_session_handler
class Rtcr::Pd_session_handler_factory : public Session_handler_factory
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
	    Ram_session_handler *ram_session_handler = NULL;
	    while (!ram_session_handler && session_handler)
		{
		    // try to cast to Pd_session_handler
		    ram_session_handler = dynamic_cast<Ram_session_handler*>(session_handler);
		    session_handler = session_handler->next();
		}

	    if(!ram_session_handler)
		Genode::error("No RAM session handler found! ");

	    /* create Session Handler for PD session */
	    return new (md_alloc) Pd_session_handler(env,
						     md_alloc,
						     ep,
						     label,
						     bootstrap,
						     ram_session_handler.session());
	}
    
    char const* name()
	{
	    return "PD";
	}
};




#endif /* _RTCR_CPU_SESSION_HANDLER_H_ */
