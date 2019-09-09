/*
 * \brief  Child creation
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/child.h>

#include <base/rpc_server.h>
#include <base/session_label.h>
#include <util/arg_string.h>
#include <base/session_label.h>


#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("orange");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;214m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;


Child::Child(Genode::Env &env,
						   Genode::Allocator &alloc,
						   const char* name,
						   Genode::Service_registry &parent_services,
						   Init_module &module)
	:
	_name (name),
	_env (env),
	_alloc (alloc),
	_module(module),
	_parent_services(parent_services),
	_pd_service(module.pd_service()),
	_cpu_service(module.cpu_service()),
	_ram_service(module.ram_service()),
	_ram_session(create_ram_session()),
	_pd_session(create_pd_session()),	
	_cpu_session(create_cpu_session()),
	_binary_rom(env, name),
	_binary_rom_ds(_binary_rom.dataspace()),
	_entrypoint(&_cap_session,
				ENTRYPOINT_STACK_SIZE,
				"entrypoint",
				false,
				_affinity_location), /* TODO: FJO still necessary? */
	_address_space(_pd_session.address_space()),
	_initial_thread(_cpu_session,
					_pd_session.cap(),
					_name),
	_child(_binary_rom_ds,
		   Genode::Dataspace_capability(),
		   _pd_session.cap(), _pd_session,
		   _ram_session.cap(), _ram_session,
		   _cpu_session.cap(),
		   _initial_thread,
		   _env.rm(),
		   _address_space,
		   _entrypoint,
		   *this,
		   *_pd_service,
		   *_ram_service,
		   *_cpu_service)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
}


void Child::start()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	_entrypoint.activate();
}


Genode::Service *Child::resolve_session_request(const char *service_name,
													   const char *args)
{
	Genode::Service *service = _module.resolve_session_request(service_name, args);

	/* Service known from parent? */
	if(!service) service = _parent_services.find(service_name);

	Genode::log("service name=",service_name, "found by parent ", service);
	/* Service not known, cannot intercept it */
	if(!service) {
		service = new (_alloc) Genode::Parent_service(service_name);
		_parent_services.insert(service);
		Genode::warning("Unknown service: ", service_name);
	}
	
	
	return service;
}


void Child::filter_session_args(const char *service,
									   char *args,
									   Genode::size_t args_len)
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
	Genode::Session_label const old_label = Genode::label_from_args(args);
	if (old_label == "") {
		Genode::Arg_string::set_arg_string(args, args_len, "label", _name);
	} else {
		Genode::Session_label const name(_name);
		Genode::Session_label const new_label = prefixed_label(name, old_label);
		Genode::Arg_string::set_arg_string(args, args_len, "label", new_label.string());
	}
}


Cpu_session &Child::create_cpu_session()
{
	DEBUG_THIS_CALL;	
	/* Preparing argument string */
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
					 "priority=0x%x, ram_quota=%u, label=\"%s\"",
					 Genode::Cpu_session::DEFAULT_PRIORITY, 128*1024, _name);

	/* Issuing session method of Cpu_root */
	Genode::Session_capability cpu_cap = _cpu_service->session(args_buf,Genode::Affinity());
	
	Cpu_session *cpu_session = static_cast<Cpu_session*>(_module.child_info(_name)->cpu_session);
	if(!cpu_session) {
		Genode::error("Creating custom CPU session failed: "
					  "Could not find PD session in PD root");
		throw Genode::Exception();
	}

	return *cpu_session;
}


Pd_session &Child::create_pd_session()
{
	DEBUG_THIS_CALL;	
	/* Preparing argument string */
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf), "ram_quota=%u, label=\"%s\"", 20*1024*sizeof(long), _name);
	
	/* Issuing session method of pd_root */
	Genode::Session_capability pd_cap = _pd_service->session(args_buf, Genode::Affinity());
	Pd_session *pd_session = static_cast<Pd_session *>(_module.child_info(_name)->pd_session);
	
	if(!pd_session) {
		Genode::error("Creating custom PD session failed: Could",
					  " not find PD session in PD root");
		throw Genode::Exception();
	}

	return *pd_session;
}


Ram_session &Child::create_ram_session()
{
	DEBUG_THIS_CALL;
	/* Preparing argument string */
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
					 "ram_quota=%u, phys_start=0x%lx, phys_size=0x%lx, label=\"%s\"",
					 4*1024*sizeof(long), 0UL, 0UL, _name);

	/* Issuing session method of Ram_root */
	Genode::Session_capability ram_cap = _ram_service->session(args_buf,Genode::Affinity());

	/* Find created RPC object in Ram_root's list */
	Ram_session *ram_session = static_cast<Ram_session *>(_module.child_info(_name)->ram_session);
	Genode::log("b");	
	if(!ram_session) {
		Genode::error("Creating custom RAM session failed",
					  ": Could not find RAM session in RAM root");
		throw Genode::Exception();
	}

	return *ram_session;
}
