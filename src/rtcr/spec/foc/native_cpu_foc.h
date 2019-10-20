#include <foc_native_cpu/client.h>
#include <foc_native_cpu/foc_native_cpu.h>
#include <base/rpc_server.h>

#include <rtcr/cpu/cpu_session.h>
#include <rtcr/cpu/cpu_thread.h>

namespace Rtcr {
	class Native_cpu_component;
}


using namespace Rtcr;


class Rtcr::Native_cpu_component : public Genode::Rpc_object<Genode::Foc_native_cpu,Rtcr::Native_cpu_component>
{
	private:

	    Cpu_session &_cpu_session_component;
	Genode::Foc_native_cpu_client  _foc_native_cpu;

	public:

		Native_cpu_component(Cpu_session &cpu_session_component)
		: _cpu_session_component(cpu_session_component),
		  _foc_native_cpu(_cpu_session_component.real_cpu_session().native_cpu())
		{
			_cpu_session_component.thread_ep().manage(this);
		}

		~Native_cpu_component()
		{
			_cpu_session_component.thread_ep().dissolve(this);
		}

	Genode::Native_capability native_cap(Genode::Thread_capability thread_cap) override
		{
			auto lambda = [&] (Cpu_thread *cpu_thread) {
				return _foc_native_cpu.native_cap(cpu_thread->parent_cap());
			};

			return _cpu_session_component.thread_ep().apply(thread_cap, lambda);
		}

	Genode::Foc_thread_state thread_state(Genode::Thread_capability cap) override
		{
			auto lambda = [&] (Cpu_thread *cpu_thread) {
				return _foc_native_cpu.thread_state(cpu_thread->parent_cap());
			};

			return _cpu_session_component.thread_ep().apply(cap, lambda);
		}
};
