/*
 * \brief  Base Module creates all intecepting sessions
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_INIT_MODULE_H_
#define _RTCR_INIT_MODULE_H_

/* Genode includes */
#include <base/allocator.h>
#include <base/service.h>
#include <util/list.h>

/* Local includes */
#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/ram/ram_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>
#include <rtcr/child_info.h>

namespace Rtcr {
	class Init_module;
	typedef Genode::String<16> Module_name;	
}


using namespace Rtcr;

/**
 * The class Rtcr::Init_module provides the intercepting sessions and handles
 * the communication between a target child and the corresponding sessions.
 */
class Rtcr::Init_module
{
protected:
	/**
	 * Entrypoint for managing child's resource-sessions (PD, CPU, RAM)
	 */
	Genode::Entrypoint _ep;
	Genode::Allocator &_alloc;
	Genode::Env &_env;

	Genode::Lock _childs_lock;
	Genode::List<Child_info> _childs;
	
	/* intercepting RAM session. The order of following declarations defines the
	 * order of initialization. _pd_root depends on _ram_session! */
	Ram_root *_ram_root;
	Genode::Local_service *_ram_service;
	
	/* intercepting PD session */
	Pd_root *_pd_root;
	Genode::Local_service *_pd_service;	

	/* intercepting CPU session. _cpu_root depends on _pd_root.*/
	Cpu_root *_cpu_root;
	Genode::Local_service *_cpu_service;	

	/* intercepting RM session. _rm_root depends on _ram_session.*/
	Rm_root *_rm_root;
	Genode::Local_service *_rm_service;	

	/* intercepting ROM session */
	Rom_root *_rom_root;
	Genode::Local_service *_rom_service;	

	/* intercepting Log session */
	Log_root *_log_root;
	Genode::Local_service *_log_service;
	
	/* intercepting Timer session */
	Timer_root *_timer_root;
	Genode::Local_service *_timer_service;	

	bool _parallel;
	inline bool read_parallel();

	void checkpoint(Child_info *child);

	void init(Pd_root *pd_root);
	void init(Ram_root *ram_root);	
	void init(Cpu_root *cpu_root);	
	void init(Rom_root *rom_root);	
	void init(Rm_root *rm_root);	
	void init(Timer_root *timer_root);	
	void init(Log_root *log_root);	
	
public:  

	Init_module(Genode::Env &env, Genode::Allocator &alloc);
	
	~Init_module();


	Genode::Service *ram_service() { return _ram_service; }
	Genode::Service *cpu_service() { return _cpu_service; }
	Genode::Service *pd_service() { return _pd_service; }

	Child_info *child_info(const char* name);
	Genode::List<Child_info> *childs() { return &_childs; }
	void pause();
	void resume();
	void checkpoint();
	bool ready();

	Genode::Service *resolve_session_request(const char *service_name,
											 const char *args);

	/**
	 * Name of your module.
	 *
	 * \return Name of your module
	 */
	virtual Module_name name() = 0;
	
};

#endif /* _RTCR_BASE_MODULE_H_ */
