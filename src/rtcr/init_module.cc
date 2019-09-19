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

	_env(env),
	_alloc(alloc),
	_config(env, "config"),
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

}


bool Init_module::read_parallel()
{
	try {
		Genode::Xml_node child_node = _config.xml().sub_node("checkpoint");
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
	Pd_session::Pd_checkpointable &pd = static_cast<Pd_session*>(child->pd_session)->pd_checkpointable;
	Pd_session::Ram_checkpointable &ram = static_cast<Pd_session*>(child->pd_session)->ram_checkpointable;

	Cpu_session *cpu_session = static_cast<Cpu_session*>(child->cpu_session);
	Rm_session *rm_session = static_cast<Rm_session*>(child->rm_session);
	Rom_session *rom_session = static_cast<Rom_session*>(child->rom_session);
	Timer_session *timer_session = static_cast<Timer_session*>(child->timer_session);
	Log_session *log_session = static_cast<Log_session*>(child->log_session);
	Capability_mapping *capability_mapping = child->capability_mapping;

	if(_parallel) {
		/* start all checkpointing threads */
		capability_mapping->start_checkpoint();

		pd.start_checkpoint();
		ram.start_checkpoint();
		cpu_session->start_checkpoint();

		if(rm_session) rm_session->start_checkpoint();
		if(rom_session) rom_session->start_checkpoint();
		if(log_session) log_session->start_checkpoint();
		if(timer_session) timer_session->start_checkpoint();

		/* wait until all threads finished */
		capability_mapping->join_checkpoint();
		pd.join_checkpoint();
		ram.join_checkpoint();
		cpu_session->join_checkpoint();

		if(rm_session) rm_session->join_checkpoint();
		if(rom_session) rom_session->join_checkpoint();
		if(log_session) log_session->join_checkpoint();
		if(timer_session) timer_session->join_checkpoint();
	} else {
		/* start & wait for pd_session */
		pd.start_checkpoint();
		pd.join_checkpoint();

		ram.start_checkpoint();
		ram.join_checkpoint();
		
		/* start & wait for cpu_session */
		cpu_session->start_checkpoint();
		cpu_session->join_checkpoint();

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
