/*
 * \brief  Child creation
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/target_child.h>

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


Target_child::Target_child(Genode::Env &env,
						   Genode::Allocator &alloc,
						   Genode::Service_registry &parent_services,
						   const char* label,
						   Module &module)
	:
	_name (label),
	_env (env),
	_parallel(false),
	_alloc (alloc),
	_module(module),
	_ram_service("RAM", &module.ram_root()),
	_ram_session(_find_ram_session(label, module.ram_root())),
	_pd_service("PD", &module.pd_root()),
	_pd_session(_find_pd_session(label, module.pd_root())),
	_cpu_service("CPU", &module.cpu_root()),
	_cpu_session(_find_cpu_session(label, module.cpu_root())),
	_rm_service("RM", &module.rm_root()),
	_rom_service("ROM", &module.rom_root()),
	_rom_connection(env, label),
	_log_service("LOG", &module.log_root()),
	_timer_service("Timer", &module.timer_root()),
	_capability_mapping(env, alloc, _pd_session),
	_entrypoint(&_cap_session,
				ENTRYPOINT_STACK_SIZE,
				"entrypoint",
				false,
				_affinity_location), /* TODO: FJO still necessary? */
	_in_bootstrap    (true),
	_parent_services (parent_services),
	_address_space(_pd_session.address_space()),
	_initial_thread(_cpu_session,
					_pd_session.cap(),
					_name.string()),
	_child(_rom_connection.dataspace(),
		   Genode::Dataspace_capability(),
		   _pd_session.cap(),
		   _pd_session,
		   _ram_session.cap(),
		   _ram_session,
		   _cpu_session.cap(),
		   _initial_thread,
		   _env.rm(),
		   _address_space,
		   _entrypoint,
		   *this,
		   _pd_service,
		   _ram_service,
		   _cpu_service)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

#ifdef VERBOSE		
		Genode::log("Execute checkpointable ",_parallel ? "parallel" : "sequential");
#endif
}


void Target_child::start()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	_entrypoint.activate();
}

void Target_child::pause()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	_cpu_session.pause();
}

void Target_child::resume()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	_cpu_session.resume();	
}


void Target_child::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
	if(_parallel) {
		/* start all checkpointing threads */
		_capability_mapping.start_checkpoint();
		_pd_session.start_checkpoint();
		_cpu_session.start_checkpoint();
		_ram_session.start_checkpoint();
		if(_rm_service.session()) _rm_service.session()->start_checkpoint();
		if(_rom_service.session()) _rom_service.session()->start_checkpoint();
		if(_log_service.session()) _log_service.session()->start_checkpoint();
		if(_timer_service.session()) _timer_service.session()->start_checkpoint();
		
		/* wait until all threads finished */
		_pd_session.join_checkpoint();
		_cpu_session.join_checkpoint();
		_ram_session.join_checkpoint();
		if(_rm_service.session()) _rm_service.session()->join_checkpoint();
		if(_rom_service.session()) _rom_service.session()->join_checkpoint();
		if(_log_service.session()) _log_service.session()->join_checkpoint();
		if(_timer_service.session()) _timer_service.session()->join_checkpoint();
		
		_capability_mapping.join_checkpoint();
	} else {
		_pd_session.start_checkpoint();
		_pd_session.join_checkpoint();
    
		_cpu_session.start_checkpoint();
		_cpu_session.join_checkpoint();    

		_ram_session.start_checkpoint();
		_ram_session.join_checkpoint();

		if(_rm_service.session()) {
			_rm_service.session()->start_checkpoint();
			_rm_service.session()->join_checkpoint();
		}
		if(_rom_service.session()) {
			_rom_service.session()->start_checkpoint();
			_rom_service.session()->join_checkpoint();				
		}
		if(_log_service.session()) {
			_log_service.session()->start_checkpoint();
			_log_service.session()->join_checkpoint();			
		}
		if(_timer_service.session()) {
			_timer_service.session()->start_checkpoint();
			_timer_service.session()->join_checkpoint();			
		}
		
		_capability_mapping.start_checkpoint();
		_capability_mapping.join_checkpoint();    
	}
}


void Target_child::print(Genode::Output &output) const {
	_pd_session.print(output);
	_cpu_session.print(output);
	_ram_session.print(output);
	// if(_rm_service.session()) _rm_service.session()->print(output);
	// if(_rom_service.session()) _rom_service.session()->print(output);
	// if(_log_service.session()) _log_service.session()->print(output);
	// if(_timer_service.session()) _timer_service.session()->print(output);		
}


Genode::Service *Target_child::resolve_session_request(const char *service_name, const char *args)
{
#ifdef DEBUG
	Genode::log("\033[36m", __func__,"(",service_name, ", ", args, ")", "\033[0m");
#endif


	if(!Genode::strcmp(service_name, "LOG") && _in_bootstrap) {
#ifdef DEBUG		
		Genode::log("  Unsetting bootstrap_phase");
#endif		
		_in_bootstrap = false;
	}
	
	/* Service known from parent? */
	Genode::Service *service = _parent_services.find(service_name);
	if(service) {
		return service;
	}

	if(!Genode::strcmp(service_name, "PD")) {
		return &_pd_service;
	} else if(!Genode::strcmp(service_name, "CPU")) {
		return &_cpu_service;
	} else if(!Genode::strcmp(service_name, "RAM")) {
		return &_ram_service;
	} else if(!Genode::strcmp(service_name, "RM")) {
		return &_rm_service;	
	} else if(!Genode::strcmp(service_name, "ROM")) {
		return &_rom_service;
	} else if(!Genode::strcmp(service_name, "LOG")) {
		return &_log_service;
	} else if(!Genode::strcmp(service_name, "Timer")) {
		return &_timer_service;			
	}

	/* Service not known, cannot intercept it */
	if(!service) {
		service = new (_alloc) Genode::Parent_service(service_name);
		_parent_services.insert(service);
		Genode::warning("Unknown service: ", service_name);
	}

	return service;
}


void Target_child::filter_session_args(const char *service,
									   char *args,
									   Genode::size_t args_len)
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
	Genode::Session_label const old_label = Genode::label_from_args(args);
	if (old_label == "") {
		Genode::Arg_string::set_arg_string(args, args_len, "label", _name.string());
	} else {
		Genode::Session_label const name(_name.string());
		Genode::Session_label const new_label = prefixed_label(name, old_label);
		Genode::Arg_string::set_arg_string(args, args_len, "label", new_label.string());
	}
}

Cpu_session &Target_child::_find_cpu_session(const char *label,Cpu_root &cpu_root)
{
	/* Preparing argument string */
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
					 "priority=0x%x, ram_quota=%u, label=\"%s\"",
					 Genode::Cpu_session::DEFAULT_PRIORITY, 128*1024, label);
	
	/* Issuing session method of Cpu_root */
	Genode::Session_capability cpu_cap = cpu_root.session(args_buf, Genode::Affinity());

	/* Find created RPC object in Cpu_root's list */
	Cpu_session *cpu_session = cpu_root.sessions().first();
	if(cpu_session) cpu_session = cpu_session->find_by_badge(cpu_cap.local_name());
	if(!cpu_session) {
		Genode::error("Creating custom CPU session failed: "
					  "Could not find PD session in PD root");
		throw Genode::Exception();
	}

	return *cpu_session;
}


Pd_session &Target_child::_find_pd_session(const char *label, Pd_root &pd_root)
{
	/* Preparing argument string */
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf), "ram_quota=%u, label=\"%s\"", 20*1024*sizeof(long), label);
	/* Issuing session method of pd_root */
	Genode::Session_capability pd_cap = pd_root.session(args_buf, Genode::Affinity());

	/* Find created RPC object in pd_root's list */
	Pd_session *pd_session = pd_root.sessions().first();
	if(pd_session) pd_session = pd_session->find_by_badge(pd_cap.local_name());
	if(!pd_session) {
		Genode::error("Creating custom PD session failed: Could not find PD session in PD root");
		throw Genode::Exception();
	}

	return *pd_session;
}


Ram_session &Target_child::_find_ram_session(const char *label, Ram_root &ram_root)
{ 
	/* Preparing argument string */
	char args_buf[160];
	Genode::snprintf(args_buf, sizeof(args_buf),
					 "ram_quota=%u, phys_start=0x%lx, phys_size=0x%lx, label=\"%s\"",
					 4*1024*sizeof(long), 0UL, 0UL, label);

	/* Issuing session method of Ram_root */
	Genode::Session_capability ram_cap = ram_root.session(args_buf, Genode::Affinity());

	/* Find created RPC object in Ram_root's list */
	Ram_session *ram_session = ram_root.sessions().first();
	if(ram_session) ram_session = ram_session->find_by_badge(ram_cap.local_name());
	if(!ram_session) {
		Genode::error("Creating custom RAM session failed: Could not find RAM session in RAM root");
		throw Genode::Exception();
	}

	return *ram_session;
}
