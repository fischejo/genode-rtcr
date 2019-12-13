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
	_info->i_ts.r0 = ts.r0();
	_info->i_ts.r1 = ts.r1();
	_info->i_ts.r2 = ts.r2();
	_info->i_ts.r3 = ts.r3();
	_info->i_ts.r4 = ts.r4();
	_info->i_ts.r5 = ts.r5();
	_info->i_ts.r6 = ts.r6();
	_info->i_ts.r7 = ts.r7();
	_info->i_ts.r8 = ts.r8();
	_info->i_ts.r9 = ts.r9();
	_info->i_ts.r10 = ts.r10();
	_info->i_ts.r11 = ts.r11();
	_info->i_ts.r12 = ts.r12();

	_info->i_ts.sp = ts.sp();
	_info->i_ts.lr = ts.lr();
	_info->i_ts.ip = ts.ip();
	_info->i_ts.cpsr = ts.cpsr();
	_info->i_ts.cpu_exception = ts.cpu_exception();

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
	state->set_r0(_info->i_ts.r0);
	state->set_r1(_info->i_ts.r1);
	state->set_r2(_info->i_ts.r2);
	state->set_r3(_info->i_ts.r3);
	state->set_r4(_info->i_ts.r4);
	state->set_r5(_info->i_ts.r5);
	state->set_r6(_info->i_ts.r6);
	state->set_r7(_info->i_ts.r7);
	state->set_r8(_info->i_ts.r8);
	state->set_r9(_info->i_ts.r9);
	state->set_r10(_info->i_ts.r10);
	state->set_r11(_info->i_ts.r11);
	state->set_r12(_info->i_ts.r12);
	state->set_sp(_info->i_ts.sp);
	state->set_lr(_info->i_ts.lr);
	state->set_ip(_info->i_ts.ip);
	state->set_cpsr(_info->i_ts.cpsr);
	state->set_cpu_exception(_info->i_ts.cpu_exception);

	info->set_allocated_ts(state);
}
