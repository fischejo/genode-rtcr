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

	for(int i = 0; i < 6; i++) {
		Genode::print(output, "   r",i*5,"-r",i*5+5,": ",
				  Hex(i_ts.r[0+i*5], Hex::PREFIX, Hex::PAD), " ",
				  Hex(i_ts.r[1+i*5], Hex::PREFIX, Hex::PAD), " ",
				  Hex(i_ts.r[2+i*5], Hex::PREFIX, Hex::PAD), " ",
				  Hex(i_ts.r[3+i*5], Hex::PREFIX, Hex::PAD), " ",
				  Hex(i_ts.r[4+i*5], Hex::PREFIX, Hex::PAD), "\n");
	}

	Genode::print(output, "   sp, ip: ",
				  Hex(i_ts.sp, Hex::PREFIX, Hex::PAD), " ",
				  Hex(i_ts.ip, Hex::PREFIX, Hex::PAD)), "\n";
}
