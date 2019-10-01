/*
 * \brief  Checkpointing capabilities
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */


#include <rtcr/cap/capability_mapping.h>


using namespace Rtcr;


Capability_mapping::Capability_mapping(Genode::Env &env,
                                       Genode::Allocator &alloc,
                                       Pd_session *pd_session)
	:
	_env(env),
	_alloc(alloc),
	Checkpointable(env, "capability_mapping") {}

Capability_mapping::~Capability_mapping() {}


void Capability_mapping::checkpoint() {}


void Capability_mapping::print(Genode::Output &output) const {
	Genode::print(output, " Capability map: not supported by foc\n");
}


Genode::addr_t Capability_mapping::find_kcap_by_badge(Genode::uint16_t badge)
{
	return badge;
}
