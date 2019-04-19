/*
 * \brief  Target's state
 * \author Denis Huber
 * \date   2016-10-25
 */

#ifndef _RTCR_TARGET_STATE_H_
#define _RTCR_TARGET_STATE_H_

/* Genode includes */
#include <util/list.h>
#include <base/heap.h>

/* Rtcr includes */
#include <rtcr_core/cpu/stored_cpu_session_info.h>
#include <rtcr_core/pd/stored_pd_session_info.h>
#include <rtcr_core/ram/stored_ram_session_info.h>
#include <rtcr_core/rm/stored_rm_session_info.h>
#include <rtcr_core/rom/stored_rom_session_info.h>

#include <rtcr_timer/stored_timer_session_info.h>
#include <rtcr_log/stored_log_session_info.h>

namespace Rtcr {
    class Target_state;
}

class Rtcr::Target_state
{

private:

    Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge,
							Genode::List<Stored_ram_session_info> &state_infos);

    Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge,
							Genode::List<Stored_pd_session_info> &state_infos);

    Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge,
							Genode::List<Stored_rm_session_info> &state_infos);

    Genode::Dataspace_capability _find_stored_dataspace(Genode::uint16_t badge,
							Genode::List<Stored_attached_region_info> &state_infos);

  
public:
    Genode::Env       &_env;
    Genode::Allocator &_alloc;
  
    Genode::List<Stored_pd_session_info>    _stored_pd_sessions;
    Genode::List<Stored_cpu_session_info>   _stored_cpu_sessions;
    Genode::List<Stored_ram_session_info>   _stored_ram_sessions;
    Genode::List<Stored_rom_session_info>   _stored_rom_sessions;
    Genode::List<Stored_rm_session_info>    _stored_rm_sessions;
    Genode::List<Stored_log_session_info>   _stored_log_sessions;

    Genode::List<Stored_timer_session_info> _stored_timer_sessions;

    Genode::addr_t _cap_idx_alloc_addr;


    Target_state(Genode::Env &env, Genode::Allocator &alloc);
    ~Target_state();

    void print(Genode::Output &output) const;

    /**
     * Searches for a dataspace which stores the content of a child's dataspace
     * in ALL possible session RPC objects
     */
    Genode::Dataspace_capability find_stored_dataspace(Genode::uint16_t badge);

  
};

#endif /* _RTCR_TARGET_STATE_H_ */
