/*
 * \brief  Stores PD session state
 * \author Denis Huber
 * \date   2016-11-21
 */

#ifndef _RTCR_PD_SESSION_INFO_H_
#define _RTCR_PD_SESSION_INFO_H_

/* Genode includes */
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>
#include <rtcr_core/pd/native_capability_info.h>
#include <rtcr_core/pd/signal_context_info.h>
#include <rtcr_core/pd/signal_source_info.h>

namespace Rtcr {
	struct Pd_session_info;
}

/**
 * State information about a PD session
 */
struct Rtcr::Pd_session_info : Session_rpc_info
{
	/**
	 * Lock for Signal_sources
	 */
	Genode::Lock                         signal_sources_lock;
	/**
	 * List for monitoring the creation and destruction of Signal_source_capabilities
	 */
	Genode::List<Signal_source_info>     signal_sources;
	/**
	 * Lock for Signal_contexts
	 */
	Genode::Lock                         signal_contexts_lock;
	/**
	 * List for monitoring the creation and destruction of Signal_context_capabilities
	 */
	Genode::List<Signal_context_info>    signal_contexts;
	/**
	 * Lock for Native_capabilities
	 */
	Genode::Lock                         native_caps_lock;
	/**
	 * List for monitoring the creation and destruction of Native_capabilities
	 */
	Genode::List<Native_capability_info> native_caps;

	Pd_session_info(const char* creation_args, bool bootstrapped)
		:
		Session_rpc_info(creation_args, "", bootstrapped),
		signal_sources_lock(), signal_sources(),
		signal_contexts_lock(), signal_contexts(),
		native_caps_lock(), native_caps()
	{ }

	void print(Genode::Output &output) const
	{
		using Genode::Hex;

		Session_rpc_info::print(output);
	}
};

#endif /* _RTCR_PD_SESSION_INFO_H_ */
