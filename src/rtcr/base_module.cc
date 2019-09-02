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


Base_module::Base_module(Genode::Env &env, Genode::Allocator &alloc)
	:
	_ep(env, 16*1024, "resources ep"),
	_ram_root(env, alloc, _ep, _childs_lock, _childs),
	_pd_root(env, alloc, _ep,  _childs_lock, _childs),
	_cpu_root(env, alloc, _ep, _childs_lock, _childs),	
	_rm_root(env, alloc, _ep,  _childs_lock, _childs),
	_rom_root(env, alloc, _ep, _childs_lock, _childs),
	_log_root(env, alloc, _ep, _childs_lock, _childs),
	_timer_root(env, alloc, _ep, _childs_lock, _childs),
	_ram_service("RAM", &_ram_root),
	_pd_service("PD", &_pd_root),
	_cpu_service("CPU", &_cpu_root),
	_rm_service("RM", &_rm_root),
	_rom_service("ROM", &_rom_root),
	_timer_service("Timer", &_timer_root),
	_log_service("LOG", &_log_root),
	_parallel(read_parallel())	
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;
	env.parent().announce(_ep.manage(_pd_root));
	env.parent().announce(_ep.manage(_ram_root));
	env.parent().announce(_ep.manage(_cpu_root));
	env.parent().announce(_ep.manage(_rm_root));
	env.parent().announce(_ep.manage(_rom_root));
	env.parent().announce(_ep.manage(_timer_root));
	env.parent().announce(_ep.manage(_log_root));		
}


bool Base_module::read_parallel()
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


Child_info *Base_module::child_info(const char* name)
{
	Child_info *child = _childs.first();
	if(child) child = child->find_by_name(name);
	return child;
}


void Base_module::pause(Child_info *child)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;	
	child->cpu_session->pause();
}


void Base_module::pause()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

	_childs_lock.lock();
	Child_info *child = _childs.first();
	while(child) {
		pause(child);
		child = child->next();
	}
	_childs_lock.unlock();
}


void Base_module::resume(Child_info *child)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;		
	child->cpu_session->resume();	
}

void Base_module::resume()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;	

	_childs_lock.lock();
	Child_info *child = _childs.first();
	while(child) {
		resume(child);
		child = child->next();
	}
	_childs_lock.unlock();	
}


void Base_module::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;	

	Child_info *child = _childs.first();
	while(child) {
		checkpoint(child);
		child = child->next();
	}
}


void Base_module::checkpoint(Child_info *child)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;
		
	if(_parallel) {
		/* start all checkpointing threads */
		child->capability_mapping->start_checkpoint();

		child->pd_session->start_checkpoint();
		child->cpu_session->start_checkpoint();
		child->ram_session->start_checkpoint();
		
		if(child->rm_session) child->rm_session->start_checkpoint();
		if(child->rom_session) child->rom_session->start_checkpoint();
		if(child->log_session) child->log_session->start_checkpoint();
		if(child->timer_session) child->timer_session->start_checkpoint();

		/* wait until all threads finished */
		child->pd_session->join_checkpoint();
		child->cpu_session->join_checkpoint();
		child->ram_session->join_checkpoint();

		if(child->rm_session) child->rm_session->join_checkpoint();
		if(child->rom_session) child->rom_session->join_checkpoint();
		if(child->log_session) child->log_session->join_checkpoint();		
		if(child->timer_session) child->timer_session->join_checkpoint();		   
		
		child->capability_mapping->join_checkpoint();
	} else {
		child->pd_session->start_checkpoint();
		child->pd_session->join_checkpoint();
		
		child->cpu_session->start_checkpoint();
		child->cpu_session->join_checkpoint();
		
		child->ram_session->start_checkpoint();
		child->ram_session->join_checkpoint();
		
		if(child->rm_session) child->rm_session->start_checkpoint();
		if(child->rm_session) child->rm_session->join_checkpoint();
		
		if(child->rom_session) child->rom_session->start_checkpoint();
		if(child->rom_session) child->rom_session->join_checkpoint();
		
		if(child->log_session) child->log_session->start_checkpoint();
		if(child->log_session) child->log_session->join_checkpoint();		

		if(child->timer_session) child->timer_session->start_checkpoint();
		if(child->timer_session) child->timer_session->join_checkpoint();	  

		child->capability_mapping->start_checkpoint();		
		child->capability_mapping->join_checkpoint();
	}
}


Genode::Service *Base_module::resolve_session_request(const char *service_name,
													   const char *args)
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL;

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
	return 0;
}
