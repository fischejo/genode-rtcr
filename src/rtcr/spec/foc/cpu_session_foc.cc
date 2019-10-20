/*
 * \brief  Intercepting Cpu session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/cpu/cpu_session.h>
#include "native_cpu_foc.h"

using namespace Rtcr;

void Cpu_session::deploy_queue(Genode::Dataspace_capability ds) {}
void Cpu_session::rq(Genode::Dataspace_capability ds) {}
void Cpu_session::dead(Genode::Dataspace_capability ds) {}
void Cpu_session::killed() {}

Genode::Capability<Cpu_session::Native_cpu> Cpu_session::_setup_native_cpu()
{
	Native_cpu_component *native_cpu_component =
		new (_md_alloc) Native_cpu_component(*this);

	return native_cpu_component->cap();
}


void Cpu_session::_cleanup_native_cpu()
{
	Native_cpu_component *native_cpu_component = nullptr;
    thread_ep().apply(_native_cpu_cap, [&] (Native_cpu_component *c) { native_cpu_component = c; });

    if (!native_cpu_component) return;

	destroy(_md_alloc, native_cpu_component);
}
