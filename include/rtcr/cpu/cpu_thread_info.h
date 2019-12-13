/*
 * \brief  Intercepting Cpu thread
 * \author Denis Huber
 * \date   2016-10-21
 */

#ifndef _RTCR_CPU_THREAD_INFO_H_
#define _RTCR_CPU_THREAD_INFO_H_

/* Genode inlcudes */
#include <base/rpc_server.h>
#include <util/fifo.h>
#include <util/list.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>

namespace Rtcr {
	class Cpu_thread_info;
}


class Rtcr::Cpu_thread_info : public Normal_info,
                              public Genode::List<Cpu_thread_info>::Element,
                              public Genode::Fifo<Cpu_thread_info>::Element
{
public:
	using Genode::List<Cpu_thread_info>::Element::next;
	
	Genode::uint16_t i_pd_session_badge;
	Genode::Cpu_session::Name i_name;
	Genode::Cpu_session::Weight i_weight;
	Genode::addr_t i_utcb;
	bool i_started;
	bool i_paused;
	bool i_single_step;
	Genode::Affinity::Location i_affinity;
	Genode::uint16_t i_sigh_badge;
	Genode::Thread_state i_ts;
	bool bootstrapped;

	
	Cpu_thread_info *find_by_badge(Genode::uint16_t badge) {
		if(badge == i_badge)
			return this;
		Cpu_thread_info *obj = next();
		return obj ? obj->find_by_badge(badge) : 0;
	}

	
	Cpu_thread_info *find_by_name(const char* name) {
		if(!Genode::strcmp(name, i_name.string()))
			return this;
		Cpu_thread_info *obj = next();
		return obj ? obj->find_by_name(name) : 0;
	}

	Cpu_thread_info() {};
	
	Cpu_thread_info(const char *name,
	                Genode::Cpu_session::Weight weight,
	                Genode::addr_t utcb,
	                bool bootstrapped)
		: i_name(name), i_weight(weight), i_utcb(utcb), bootstrapped(bootstrapped) {}
		

	void print(Genode::Output &output) const;
};




#endif /* _RTCR_CPU_THREAD_INFO_H_ */
