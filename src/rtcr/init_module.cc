/*
 * \brief Init module
 * \author Johannes Fischer
 * \date   2019-08-29
 */
#include <rtcr/init_module.h>

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


Init_module::Init_module(Genode::Env &env, Genode::Allocator &alloc)
	:
	_ep(env, 16*1024, "resources ep"),	
	_env(env),
	_alloc(alloc),
	_parallel(read_parallel())
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;
	// env.parent().announce(_ep.manage(_pd_root));
	// env.parent().announce(_ep.manage(_ram_root));
	// env.parent().announce(_ep.manage(_cpu_root));
	// env.parent().announce(_ep.manage(_rm_root));
	// env.parent().announce(_ep.manage(_rom_root));
	// env.parent().announce(_ep.manage(_timer_root));
	// env.parent().announce(_ep.manage(_log_root));		
}


Init_module::~Init_module()
{
	Genode::destroy(_alloc, _pd_root);
	Genode::destroy(_alloc, _ram_root);
	Genode::destroy(_alloc, _cpu_root);
	Genode::destroy(_alloc, _rm_root); 	
	Genode::destroy(_alloc, _rom_root); 	
	Genode::destroy(_alloc, _timer_root); 	
	Genode::destroy(_alloc, _log_root);
	Genode::destroy(_alloc, _pd_service);
	Genode::destroy(_alloc, _ram_service);
	Genode::destroy(_alloc, _cpu_service);
	Genode::destroy(_alloc, _rm_service); 	
	Genode::destroy(_alloc, _rom_service); 	
	Genode::destroy(_alloc, _timer_service); 	
	Genode::destroy(_alloc, _log_service);
	
}

void Init_module::init(Pd_root *root)
{
	_pd_root = root;
	_pd_service = new(_alloc) Genode::Local_service("PD", root);		
}

void Init_module::init(Cpu_root *root)
{
	_cpu_root = root;
	_cpu_service = new(_alloc) Genode::Local_service("CPU", root);	
}

void Init_module::init(Ram_root *root)
{
	_ram_root = root;
	_ram_service = new(_alloc) Genode::Local_service("RAM", root);	
}

void Init_module::init(Rom_root *root)
{
	_rom_root = root;
	_rom_service = new(_alloc) Genode::Local_service("ROM", root);	
}
 
void Init_module::init(Rm_root *root)
{
	_rm_root = root;
	_rm_service = new(_alloc) Genode::Local_service("RM", root);	
}

void Init_module::init(Log_root *root)
{
	_log_root = root;
	_log_service = new(_alloc) Genode::Local_service("LOG", root);	
}

void Init_module::init(Timer_root *root)
{
	_timer_root = root;
	_timer_service = new(_alloc) Genode::Local_service("Timer", root);	
}

				
bool Init_module::read_parallel()
{
	try {
	  Genode::Xml_node child_node = Genode::config()->xml_node().sub_node("checkpoint");
	  bool const parallel = child_node.attribute_value<bool>("parallel", false);
	  Genode::log("Running Checkpointable in parallel: ", parallel);
	  return parallel;
	} catch(...) {
		Genode::warning("No parallel configured.");
	}
	return false;	
}


Child_info *Init_module::child_info(const char* name)
{
	Child_info *child = _childs.first();
	if(child) child = child->find_by_name(name);
	return child;
}


void Init_module::pause()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	_childs_lock.lock();
	Child_info *child = _childs.first();
	while(child) {
		static_cast<Cpu_session*>(child->cpu_session)->pause();		
		child = child->next();
	}
	_childs_lock.unlock();
}


void Init_module::resume()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;	

	_childs_lock.lock();
	Child_info *child = _childs.first();
	while(child) {
		static_cast<Cpu_session*>(child->cpu_session)->resume();
		child = child->next();
	}
	_childs_lock.unlock();	
}


void Init_module::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;	

	Child_info *child = _childs.first();
	while(child) {
		checkpoint(child);
		child = child->next();
	}
}


void Init_module::checkpoint(Child_info *child)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	/* well...casting is not that efficent, but due to the design of
	 * *_info object handling..this is necessary */
	Cpu_session *cpu_session = static_cast<Cpu_session*>(child->cpu_session);
	Pd_session *pd_session = static_cast<Pd_session*>(child->pd_session);
	Ram_session *ram_session = static_cast<Ram_session*>(child->ram_session);
	Rm_session *rm_session = static_cast<Rm_session*>(child->rm_session);
	Rom_session *rom_session = static_cast<Rom_session*>(child->rom_session);
	Timer_session *timer_session = static_cast<Timer_session*>(child->timer_session);
	Log_session *log_session = static_cast<Log_session*>(child->log_session);
	Capability_mapping *capability_mapping = child->capability_mapping;
	
	if(_parallel) {
		/* start all checkpointing threads */
		capability_mapping->start_checkpoint();

		pd_session->start_checkpoint();
		cpu_session->start_checkpoint();
		ram_session->start_checkpoint();
		
		if(rm_session) rm_session->start_checkpoint();
		if(rom_session) rom_session->start_checkpoint();
		if(log_session) log_session->start_checkpoint();
		if(timer_session) timer_session->start_checkpoint();

		/* wait until all threads finished */
		pd_session->join_checkpoint();
		cpu_session->join_checkpoint();
		ram_session->join_checkpoint();
		if(rm_session) rm_session->join_checkpoint();
		if(rom_session) rom_session->join_checkpoint();
		if(log_session) log_session->join_checkpoint();		
		if(timer_session) timer_session->join_checkpoint();		   
		capability_mapping->join_checkpoint();
		
	} else {
		/* start & wait for pd_session */
		pd_session->start_checkpoint();
		pd_session->join_checkpoint();

		/* start & wait for cpu_session */		
		cpu_session->start_checkpoint();
		cpu_session->join_checkpoint();

		/* start & wait for ram_session */		
		ram_session->start_checkpoint();
		ram_session->join_checkpoint();

		/* start & wait for rm_session */		
		if(rm_session) rm_session->start_checkpoint();
		if(rm_session) rm_session->join_checkpoint();

		/* start & wait for rom_session */		
		if(rom_session) rom_session->start_checkpoint();
		if(rom_session) rom_session->join_checkpoint();

		/* start & wait for log_session */		
		if(log_session) log_session->start_checkpoint();
		if(log_session) log_session->join_checkpoint();		

		/* start & wait for timer_session */		
		if(timer_session) timer_session->start_checkpoint();
		if(timer_session) timer_session->join_checkpoint();	  

		/* start & wait for capability_mapping */		
		capability_mapping->start_checkpoint();		
		capability_mapping->join_checkpoint();
	}
}


Genode::Service *Init_module::resolve_session_request(const char *service_name,
													   const char *args)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	if(!Genode::strcmp(service_name, "PD")) {
		return _pd_service;
	} else if(!Genode::strcmp(service_name, "CPU")) {
		return _cpu_service;
	} else if(!Genode::strcmp(service_name, "RAM")) {
		return _ram_service;
	} else if(!Genode::strcmp(service_name, "RM")) {
		return _rm_service;
	} else if(!Genode::strcmp(service_name, "ROM")) {
		return _rom_service;
	} else if(!Genode::strcmp(service_name, "LOG")) {
		return _log_service;
	} else if(!Genode::strcmp(service_name, "Timer")) {
		return _timer_service;
	}
	return 0;
}
