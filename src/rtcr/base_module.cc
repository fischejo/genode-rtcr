/*
 * \brief Base module
 * \author Johannes Fischer
 * \date   2019-08-29
 */
#include <rtcr/base_module.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("violet");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;207m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;

/* Create a static instance of the Base_module_factory. This registers the
 * module */
Rtcr::Base_module_factory _base_module_factory_instance;


Base_module::Base_module(Genode::Env &env,
						 Genode::Allocator &alloc,
						 Genode::Entrypoint &ep,
						 const char* label,
						 bool &bootstrap)
	:
	_env(env),
	_alloc(alloc),
	_ep(ep),
	_config(_read_config()),
	_parallel(_read_parallel()),
	_bootstrap(bootstrap),
	_ram_root(ram_root()),
	_ram_service("RAM", &_ram_root),
	_ram_session(_find_ram_session(label, _ram_root)),
	_pd_root(pd_root()),	
	_pd_service("PD", &_pd_root),
	_pd_session(_find_pd_session(label, _pd_root)),
	_cpu_root(cpu_root()),	
	_cpu_service("CPU", &_cpu_root),
	_cpu_session(_find_cpu_session(label, _cpu_root)),
	_rm_root(rm_root()),
	_rm_service("RM", &_rm_root),
	_rom_root(rom_root()),
	_rom_service("ROM", &_rom_root),
	_rom_connection(env, label),
	_log_root(log_root()),
	_log_service("LOG", &_log_root),
	_timer_root(timer_root()),
	_timer_service("Timer", &_timer_root),
	_capability_mapping(capability_mapping())
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL

#ifdef VERBOSE		
		Genode::log("Execute checkpointable ",_parallel ? "parallel" : "sequential");
#endif
	
	/* Donate ram quota to child */
	_ram_session.ref_account(_env.ram_session_cap());
	// Note: transfer goes directly to parent's ram session
	_env.ram().transfer_quota(_ram_session.parent_cap(), _read_quota());
}


Genode::Xml_node Base_module::_read_config()
{
	return Genode::config()->xml_node().sub_node("module");
}

bool Base_module::_read_parallel()
{
	DEBUG_THIS_CALL
	try {
		Genode::Xml_node ck_node = _config.sub_node("checkpoint");
		bool const parallel = ck_node.attribute_value<bool>("parallel", false);
		return parallel;
	} catch(...) {}
	return false;
}


Genode::size_t Base_module::_read_quota()
{
	Genode::size_t quota = 512*1024; // 512 kB
	// try {
	// 	Genode::Xml_node child_node = _config->sub_node("child");
	// 	long const quota = child_node.attribute_value<long>("quota", quota);
	// 	Genode::log("Child Quota: ", quota);
	// } catch(...) {
	// 	Genode::warning("No quota configured. Set default value: 512 kB.");
	// }
	
	return quota;
}


Cpu_root &Base_module::cpu_root()
{
	DEBUG_THIS_CALL	
	return *new (_alloc) Cpu_root(_env, _alloc, _ep, _pd_root, _bootstrap, &_config);
}


Pd_root &Base_module::pd_root()
{
	DEBUG_THIS_CALL
	return *new (_alloc) Pd_root(_env, _alloc, _ep, _ram_session, _bootstrap, &_config);
}


Ram_root &Base_module::ram_root()
{
	DEBUG_THIS_CALL
	DEBUG_THIS_CALL
	  
	return *new (_alloc) Ram_root(_env, _alloc, _ep, _bootstrap, &_config);
}


Rm_root &Base_module::rm_root()
{
	DEBUG_THIS_CALL	
	return *new (_alloc) Rm_root(_env, _alloc, _ep, _ram_session,_bootstrap, &_config);
}


Rom_root &Base_module::rom_root()
{
	DEBUG_THIS_CALL	
	return *new (_alloc) Rom_root(_env, _alloc, _ep, _bootstrap, &_config);
}


Log_root &Base_module::log_root()
{
	DEBUG_THIS_CALL	
	return *new (_alloc) Log_root(_env, _alloc, _ep, _bootstrap, &_config);
}


Timer_root &Base_module::timer_root()
{
	DEBUG_THIS_CALL	
	return *new (_alloc) Timer_root(_env, _alloc, _ep,_bootstrap, &_config);
}


Capability_mapping &Base_module::capability_mapping()
{
	DEBUG_THIS_CALL	
	return *new (_alloc) Capability_mapping(_env, _alloc, _pd_session, &_config);
}


Cpu_session &Base_module::_find_cpu_session(const char *label,Cpu_root &cpu_root)
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


Pd_session &Base_module::_find_pd_session(const char *label, Pd_root &pd_root)
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


Ram_session &Base_module::_find_ram_session(const char *label, Ram_root &ram_root)
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


Base_module::~Base_module()
{
	Genode::destroy(_alloc, &_cpu_root);
	Genode::destroy(_alloc, &_ram_root);
	Genode::destroy(_alloc, &_rm_root);
	Genode::destroy(_alloc, &_pd_root);
	Genode::destroy(_alloc, &_log_root);
	Genode::destroy(_alloc, &_timer_root);
	Genode::destroy(_alloc, &_capability_mapping);  
}


void Base_module::checkpoint(bool resume)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	
	/* pause all threads */
	_cpu_session.pause();

	/* checkpoint  */
	if(_parallel) {
		/* start all checkpointing threads */
		_capability_mapping.start_checkpoint();
		_pd_session.start_checkpoint();
		_cpu_session.start_checkpoint();
		_ram_session.start_checkpoint();

		/* wait until all threads finished */
		_pd_session.join_checkpoint();
		_cpu_session.join_checkpoint();
		_ram_session.join_checkpoint();
		_capability_mapping.join_checkpoint();
    
	} else {
		_pd_session.start_checkpoint();
		_pd_session.join_checkpoint();
    
		_cpu_session.start_checkpoint();
		_cpu_session.join_checkpoint();    

		_ram_session.start_checkpoint();
		_ram_session.join_checkpoint();

		_capability_mapping.start_checkpoint();
		_capability_mapping.join_checkpoint();    
	}

	/* resume all threads */
	if(resume) _cpu_session.resume();
}


Genode::Service *Base_module::resolve_session_request(const char *service_name,
													  const char *args)
{
	DEBUG_THIS_CALL
		Genode::log("\033[36m Service Request: ", service_name, "\033[0m");
	
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
		
	} else {
		Genode::Service *service = 0;	
		return service;
	}
}

