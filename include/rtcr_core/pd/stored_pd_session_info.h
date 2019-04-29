/*
 * \brief  Structure for storing PD session information
 * \author Denis Huber
 * \date   2016-10-26
 */

#ifndef _RTCR_STORED_PD_SESSION_INFO_H_
#define _RTCR_STORED_PD_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>
#include <base/printf.h>

/* Rtcr includes */
#include <rtcr_core/pd/pd_session.h>
#include <rtcr_core/pd/stored_native_capability_info.h>
#include <rtcr_core/pd/stored_signal_context_info.h>
#include <rtcr_core/pd/stored_signal_source_info.h>

#include <rtcr_core/rm/stored_region_map_info.h>

#include <rtcr/stored_info_structs.h>

namespace Rtcr {
	struct Stored_pd_session_info;
}


struct Rtcr::Stored_pd_session_info : Stored_session_info, Genode::List<Stored_pd_session_info>::Element
{
	Genode::List<Stored_signal_context_info> stored_context_infos;
	Genode::List<Stored_signal_source_info> stored_source_infos;
	Genode::List<Stored_native_capability_info> stored_native_cap_infos;
	Stored_region_map_info stored_address_space;
	Stored_region_map_info stored_stack_area;
	Stored_region_map_info stored_linker_area;

	Stored_pd_session_info(Pd_session_component &pd_session, Genode::addr_t targets_pd_kcap,
			       Genode::addr_t targets_add_kcap, Genode::addr_t targets_sta_kcap, Genode::addr_t targets_lin_kcap)
		:
		Stored_session_info(pd_session.parent_state().creation_args.string(),
				    pd_session.parent_state().upgrade_args.string(),
				    targets_pd_kcap,
				    pd_session.cap().local_name(),
				    pd_session.parent_state().bootstrapped),
		stored_context_infos(), stored_source_infos(), stored_native_cap_infos(),
		stored_address_space(pd_session.address_space_component(), targets_add_kcap),
		stored_stack_area(pd_session.stack_area_component(), targets_sta_kcap),
		stored_linker_area(pd_session.linker_area_component(), targets_lin_kcap)
	{ }

	Stored_pd_session_info *find_by_badge(Genode::uint16_t badge)
	{
		if(badge == this->badge)
			return this;
		Stored_pd_session_info *info = next();
		return info ? info->find_by_badge(badge) : 0;
	}

	Stored_pd_session_info *find_by_bootstrapped(bool bootstrapped)
	{
		if(bootstrapped == this->bootstrapped)
			return this;
		Stored_pd_session_info *info = next();
		return info ? info->find_by_bootstrapped(bootstrapped) : 0;
	}

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Stored_session_info::print(output);
	}

};

#endif /* _RTCR_STORED_PD_SESSION_INFO_H_ */
