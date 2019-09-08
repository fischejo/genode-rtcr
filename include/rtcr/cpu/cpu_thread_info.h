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

	
	Cpu_thread_info(const char *name,
					Genode::Cpu_session::Weight weight,
					Genode::addr_t utcb,
					Genode::uint16_t badge)
		:
		Normal_info(badge),
		i_name(name),
		i_weight(weight),
		i_utcb(utcb) {}
		

	void print(Genode::Output &output) const {
		using Genode::Hex;
		Genode::print(output, "Thread:\n");
		Genode::print(output,
					  "   i_pd_session_badge=", i_pd_session_badge,
					  ", i_name=", i_name,
					  ", i_weight=", i_weight.value,
					  ", i_utcb=", Hex(i_utcb));
  
		Genode::print(output,
					  ", i_started=", i_started,
					  ", paused=", i_paused,
					  ", single_step=", i_single_step);
  
		Genode::print(output,
					  ", affinity=(", i_affinity.xpos(),
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

	
						  Cpu_thread_info *find_by_badge(Genode::uint16_t _badge) {
							  if(badge == _badge)
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

	
						  Cpu_thread_info(const char *name,
										  Genode::Cpu_session::Weight weight,
										  Genode::addr_t utcb)
							  : i_name(name), i_weight(weight), i_utcb(utcb) {}
		

						  void print(Genode::Output &output) const {
							  using Genode::Hex;
							  Genode::print(output, "Thread:\n");
							  Genode::print(output,
											"   i_pd_session_badge=", i_pd_session_badge,
											", i_name=", i_name,
											", i_weight=", i_weight.value,
											", i_utcb=", Hex(i_utcb));
  
							  Genode::print(output,
											", i_started=", i_started,
											", paused=", i_paused,
											", single_step=", i_single_step);
  
							  Genode::print(output,
											", affinity=(", i_affinity.xpos(),
											"x", i_affinity.ypos(),
											", ", i_affinity.width(),
											"x", i_affinity.height(), ")");
  
							  Genode::print(output, ", sigh_badge=", i_sigh_badge, "\n");

							  Genode::print(output, "   r0-r4: ",
											Hex(i_ts.r0, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r1, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r2, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r3, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r4, Hex::PREFIX, Hex::PAD), "\n");
  
							  Genode::print(output, "   r5-r9: ",
											Hex(i_ts.r5, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r6, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r7, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r8, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r9, Hex::PREFIX, Hex::PAD), "\n");

							  Genode::print(output, "   r10-r12: ",
											Hex(i_ts.r10, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r11, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.r12, Hex::PREFIX, Hex::PAD), "\n");

							  Genode::print(output, "   sp, lr, ip, cpsr, cpu_e: ",
											Hex(i_ts.sp, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.lr, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.ip, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.cpsr, Hex::PREFIX, Hex::PAD), " ",
											Hex(i_ts.cpu_exception, Hex::PREFIX, Hex::PAD));
							  Genode::print(output, "\n");		
						  }
					  };




#endif /* _RTCR_CPU_THREAD_INFO_H_ */
					  "x", i_affinity.ypos(),
					  ", ", i_affinity.width(),
					  "x", i_affinity.height(), ")");
  
		Genode::print(output, ", sigh_badge=", i_sigh_badge, "\n");

		Genode::print(output, "   r0-r4: ",
					  Hex(i_ts.r0, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r1, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r2, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r3, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r4, Hex::PREFIX, Hex::PAD), "\n");
  
		Genode::print(output, "   r5-r9: ",
					  Hex(i_ts.r5, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r6, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r7, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r8, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r9, Hex::PREFIX, Hex::PAD), "\n");

		Genode::print(output, "   r10-r12: ",
					  Hex(i_ts.r10, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r11, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.r12, Hex::PREFIX, Hex::PAD), "\n");

		Genode::print(output, "   sp, lr, ip, cpsr, cpu_e: ",
					  Hex(i_ts.sp, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.lr, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.ip, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.cpsr, Hex::PREFIX, Hex::PAD), " ",
					  Hex(i_ts.cpu_exception, Hex::PREFIX, Hex::PAD));
		Genode::print(output, "\n");		
	}
};




#endif /* _RTCR_CPU_THREAD_INFO_H_ */
