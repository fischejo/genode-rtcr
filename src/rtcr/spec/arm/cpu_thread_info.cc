/*
 * \brief  Intercepting Cpu thread
 * \author Johannes Fischer
 * \date   2019-12-12
 */

#include <rtcr/cpu/cpu_thread_info.h>

using namespace Rtcr;

void Cpu_thread_info::print(Genode::Output &output) const {
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
  
	Genode::print(output,", affinity=(", i_affinity.xpos(),",",
				  i_affinity.ypos(),")");
  
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
