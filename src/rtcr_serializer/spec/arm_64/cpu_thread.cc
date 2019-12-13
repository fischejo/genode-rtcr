/*
 * \brief  Serializer
 * \author Johannes Fischer
 * \date   2019-08-31
 */

#include <rtcr_serializer/serializer.h>

using namespace Rtcr;

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;207m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


Cpu_thread_info *Serializer::parse_cpu_thread(const Pb::Cpu_thread_info &info)
{
	DEBUG_THIS_CALL;

	Cpu_thread_info *_info = new(_alloc) Cpu_thread_info();
	parse_normal_info(info.normal_info(), _info);

	_info->i_pd_session_badge = info.pd_session_badge();
	_info->i_name = info.name().c_str();
	_info->i_weight = Genode::Cpu_session::Weight(info.weight());
	_info->i_utcb = info.utcb();
	_info->i_started = info.started();
	_info->i_paused = info.paused();
	_info->i_single_step = info.single_step();
	_info->i_affinity = Genode::Affinity::Location(info.affinity_x(),
	                                               info.affinity_y());

	_info->i_sigh_badge = info.sigh_badge();

	/* parse Cpu_state */
	Pb::Cpu_state ts = info.ts();
	_info->i_ts.r[0] = ts.r0();
	_info->i_ts.r[1] = ts.r1();
	_info->i_ts.r[2] = ts.r2();
	_info->i_ts.r[3] = ts.r3();
	_info->i_ts.r[4] = ts.r4();
	_info->i_ts.r[5] = ts.r5();
	_info->i_ts.r[6] = ts.r6();
	_info->i_ts.r[7] = ts.r7();
	_info->i_ts.r[8] = ts.r8();
	_info->i_ts.r[9] = ts.r9();
	_info->i_ts.r[10] = ts.r10();
	_info->i_ts.r[11] = ts.r11();
	_info->i_ts.r[12] = ts.r12();
	_info->i_ts.r[13] = ts.r13();
	_info->i_ts.r[14] = ts.r14();
	_info->i_ts.r[15] = ts.r15();
	_info->i_ts.r[16] = ts.r16();
	_info->i_ts.r[17] = ts.r17();
	_info->i_ts.r[18] = ts.r18();
	_info->i_ts.r[19] = ts.r19();
	_info->i_ts.r[20] = ts.r20();
	_info->i_ts.r[21] = ts.r21();
	_info->i_ts.r[22] = ts.r22();
	_info->i_ts.r[23] = ts.r23();
	_info->i_ts.r[24] = ts.r24();
	_info->i_ts.r[25] = ts.r25();
	_info->i_ts.r[26] = ts.r26();
	_info->i_ts.r[27] = ts.r27();
	_info->i_ts.r[28] = ts.r28();
	_info->i_ts.r[29] = ts.r29();
	_info->i_ts.sp = ts.sp();
	_info->i_ts.ip = ts.ip();

	return _info;
}


void Serializer::add_cpu_thread(Capability_mapping *_cm,
                                Pb::Cpu_session_info *cpu_session_info,
                                Cpu_thread_info *_info)
{
	DEBUG_THIS_CALL;
	/* Cpu thread info */
	Pb::Cpu_thread_info *info = cpu_session_info->add_cpu_thread_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_pd_session_badge(_info->i_pd_session_badge);
	info->set_name(_info->i_name.string());
	info->set_weight(_info->i_weight.value);
	info->set_utcb(_info->i_utcb);
	info->set_started(_info->i_started);
	info->set_paused(_info->i_paused);
	info->set_single_step(_info->i_single_step);
	info->set_affinity_x(_info->i_affinity.xpos());
	info->set_affinity_y(_info->i_affinity.ypos());
	info->set_sigh_badge(_info->i_sigh_badge);

	/* Cpu_state (also known as ts) */
	Pb::Cpu_state *state = new(_alloc) Pb::Cpu_state();
	state->set_r0(_info->i_ts.r[0]);
	state->set_r1(_info->i_ts.r[1]);
	state->set_r2(_info->i_ts.r[2]);
	state->set_r3(_info->i_ts.r[3]);
	state->set_r4(_info->i_ts.r[4]);
	state->set_r5(_info->i_ts.r[5]);
	state->set_r6(_info->i_ts.r[6]);
	state->set_r7(_info->i_ts.r[7]);
	state->set_r8(_info->i_ts.r[8]);
	state->set_r9(_info->i_ts.r[9]);
	state->set_r10(_info->i_ts.r[10]);
	state->set_r11(_info->i_ts.r[11]);
	state->set_r12(_info->i_ts.r[12]);
	state->set_r13(_info->i_ts.r[13]);
	state->set_r14(_info->i_ts.r[14]);
	state->set_r15(_info->i_ts.r[15]);
	state->set_r16(_info->i_ts.r[16]);
	state->set_r17(_info->i_ts.r[17]);
	state->set_r18(_info->i_ts.r[18]);
	state->set_r19(_info->i_ts.r[19]);
	state->set_r20(_info->i_ts.r[20]);
	state->set_r21(_info->i_ts.r[21]);
	state->set_r22(_info->i_ts.r[22]);
	state->set_r23(_info->i_ts.r[23]);
	state->set_r24(_info->i_ts.r[24]);
	state->set_r25(_info->i_ts.r[25]);
	state->set_r26(_info->i_ts.r[26]);
	state->set_r27(_info->i_ts.r[27]);
	state->set_r28(_info->i_ts.r[28]);
	state->set_r29(_info->i_ts.r[29]);
	state->set_sp(_info->i_ts.sp);
	state->set_ip(_info->i_ts.ip);

	info->set_allocated_ts(state);
}
