/*
 * \brief  State of core module
 * \author Johannes Fischer
 * \date   2019-04-23
 */

#ifndef _RTCR_CORE_STATE_H_
#define _RTCR_CORE_STATE_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/module_state.h>
#include <rtcr_core/cpu/stored_cpu_session_info.h>
#include <rtcr_core/pd/stored_pd_session_info.h>
#include <rtcr_core/ram/stored_ram_session_info.h>
#include <rtcr_core/rm/stored_rm_session_info.h>
#include <rtcr_core/rom/stored_rom_session_info.h>


namespace Rtcr {
    class Core_state;
}

using namespace Rtcr;

class Rtcr::Core_state : public virtual Module_state
{
 protected:

    Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge,
	Genode::List<Stored_ram_session_info> &state_infos);

    Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge,
	Genode::List<Stored_pd_session_info> &state_infos);

    Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge,
	Genode::List<Stored_rm_session_info> &state_infos);

    Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge,
	Genode::List<Stored_attached_region_info> &state_infos);

    void _print_pd(Genode::Output &output) const;
    void _print_cpu(Genode::Output &output) const;
    void _print_ram(Genode::Output &output) const;
    void _print_rom(Genode::Output &output) const;
    void _print_rm(Genode::Output &output) const;
    
public:
    Genode::List<Stored_pd_session_info>    _stored_pd_sessions;
    Genode::List<Stored_cpu_session_info>   _stored_cpu_sessions;
    Genode::List<Stored_ram_session_info>   _stored_ram_sessions;
    Genode::List<Stored_rom_session_info>   _stored_rom_sessions;
    Genode::List<Stored_rm_session_info>    _stored_rm_sessions;
    Genode::addr_t _cap_idx_alloc_addr;

    Core_state();
    ~Core_state();
    
    void print(Genode::Output &output) const override;

    /**
     * Searches for a dataspace which stores the content of a child's dataspace
     * in ALL possible session RPC objects
     */
    Genode::Dataspace_capability find_stored_dataspace(Genode::uint16_t badge);
};

#endif /* _RTCR_MODULE_STATE_H_ */
