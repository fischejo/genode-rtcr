/*
 * \brief  CPU Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#ifndef _RTCR_CORE_MODULE_PD_H_
#define _RTCR_CORE_MODULE_PD_H_

/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <base/service.h>
#include <base/env.h>
#include <base/child.h>
#include <base/service.h>
#include <base/snprintf.h>
#include <util/list.h


#include <rtcr_core/core_module.h>
#include <rtcr_core/pd/pd_session.h>
#include <rtcr_core/pd/kcap_badge_info.h>
#include <rtcr_core/pd/ref_badge_innfo.h>

namespace Rtcr {
    class Core_module_pd;
}

using namespace Rtcr;

class Rtcr::Core_module_pd: public Core_module_base
{
private:
    Genode::Env        &_env;
    Genode::Allocator  &_md_alloc;
    Genode::Entrypoint &_ep;

    Pd_root &_pd_root;
    Genode::Local_service &_pd_service;
    Pd_session_component   &_pd_session;

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


/* implement virtual methods of Core_module_base */
  Pd_root & pd_root() {
    return _pd_root;
  }
  
  Genode::Local_service &pd_service() {
    return _pd_service;
  }
  Pd_session_component &pd_session() {
    return _pd_session;
  }
    
public:
    Core_module_pd(Genode::Env &env,
		       Genode::Allocator &md_alloc,
		       Genode::Entrypoint &ep,
		       const char* label,
		       bool &bootstrap,
		       Ram_session_component *ram_session)

    ~Core_module_pd();

  /* implement virtual method of Core_module_base */
  Genode::addr_t find_kcap_by_badge(Genode::uint16_t badge);	
};




#endif /* _RTCR_CORE_MODULE_PD_H_ */
