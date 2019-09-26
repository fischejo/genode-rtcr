/*
 * \brief  Checkpointing capabilities
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */


#include <rtcr/cap/capability_mapping.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("blue");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;27m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;


Capability_mapping::Capability_mapping(Genode::Env &env,
                                       Genode::Allocator &alloc,
                                       Pd_session *pd_session)
	:
	_env(env),
	_alloc(alloc),
	Checkpointable(env, "capability_mapping")
{
	DEBUG_THIS_CALL;
}

Capability_mapping::~Capability_mapping() {}


void Capability_mapping::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;
}


void Capability_mapping::print(Genode::Output &output) const {
	Genode::print(output, " Capability map: not supported\n");
}


Genode::addr_t Capability_mapping::find_kcap_by_badge(Genode::uint16_t badge)
{
	return badge;
}
