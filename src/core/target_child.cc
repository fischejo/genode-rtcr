/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-08-04
 */

#include <rtcr/target_child.h>


using namespace Rtcr;


Target_child::Target_child(Genode::Env &env,
			   Genode::Allocator &md_alloc,
			   Genode::Service_registry &parent_services,
			   const char *name)
:
	_name            (name),
	_env             (env),
	_md_alloc        (md_alloc),
	_resources_ep    (_env, 16*1024, "resources ep"),
	_child_ep        (_env, 16*1024, "child ep"),
	_in_bootstrap    (true),
	_parent_services (parent_services)
{
	Genode::log("\033[33m", __func__, "\033[0m(child=", _name.string(), ")");

	// print all registered session handlers
	Session_handler_factory::print();

	// create pd_session_handler
	Session_handler* session_handler;
	session_handler = Session_handler_factory::get("PD")->create(env,
								md_alloc,
								_resources_ep,
								&_session_handlers,
								_name.string(),
								_in_bootstrap);
	_session_handlers.insert(session_handler);
	pd_handler = static_cast<Pd_session_handler*>(session_handler);

	// create cpu_session_handler
	session_handler = Session_handler_factory::get("CPU")->create(env,
								md_alloc,
								_resources_ep,
								&_session_handlers,
								_name.string(),
								_in_bootstrap);
	_session_handlers.insert(session_handler);	
	cpu_handler = static_cast<Cpu_session_handler*>(session_handler);

	// create ram_session_handler
	session_handler = Session_handler_factory::get("RAM")->create(env,
								md_alloc,
								_resources_ep,
							        &_session_handlers,
								_name.string(),
								_in_bootstrap);
	_session_handlers.insert(session_handler);	
	ram_handler = static_cast<Ram_session_handler*>(session_handler);	

	// create rm_session_handler
	session_handler = Session_handler_factory::get("RM")->create(env,
								md_alloc,
								_resources_ep,
								&_session_handlers,
								_name.string(),
								_in_bootstrap);
	_session_handlers.insert(session_handler);	
	rm_handler = static_cast<Rm_session_handler*>(session_handler);	

	// create rom_session_handler
	session_handler = Session_handler_factory::get("ROM")->create(env,
								md_alloc,
								_resources_ep,
								&_session_handlers,
								_name.string(),
								_in_bootstrap);
	_session_handlers.insert(session_handler);	
	rom_handler = static_cast<Rom_session_handler*>(session_handler);	
	
	
	// Donate ram quota to child
	// TODO Replace static quota donation with the amount of quota, the child needs
	Genode::size_t donate_quota = 1024*1024;
	ram_handler->ram_session->ref_account(env.ram_session_cap());
	// Note: transfer goes directly to parent's ram session
	env.ram().transfer_quota(ram_handler->ram_session->parent_cap(), donate_quota);
	
	// do some magic
	_address_space = new(md_alloc) Genode::Region_map_client(pd_handler->pd_session->address_space());


	_initial_thread = new(md_alloc) Genode::Child::Initial_thread(
								      *(cpu_handler->cpu_session),
								      pd_handler->pd_session->cap(),
								      _name.string());
}


Target_child::~Target_child()
{
	if(_child)
		Genode::destroy(_md_alloc, _child);

	Genode::log("\033[33m", __func__, "\033[0m() ", _name.string());
}


void Target_child::start()
{
        Genode::log("Target_child::\033[33m", __func__, "\033[0m()");

	_child = new (_md_alloc) Genode::Child ( rom_handler->rom_connection->dataspace(),
						 Genode::Dataspace_capability(),
						 pd_handler->pd_session->cap(),
						 *pd_handler->pd_session,
						 ram_handler->ram_session->cap(),
						 *ram_handler->ram_session,
						 cpu_handler->cpu_session->cap(),
						 *_initial_thread,
						 _env.rm(),
						 *_address_space,
						 _child_ep.rpc_ep(),
						 *this,
						 *pd_handler->pd_service,
						 *ram_handler->ram_service,
						 *cpu_handler->cpu_service);
}


Genode::Service *Target_child::resolve_session_request(const char *service_name,
						       const char *args)
{
        Genode::log("Target_child::\033[33m", __func__, "\033[0m(", service_name, " ", args, ")");


	if(!Genode::strcmp(service_name, "LOG") && _in_bootstrap)
	{
		Genode::log("  Unsetting bootstrap_phase");
		_in_bootstrap = false;
	}

	Genode::Service *service = 0;
	
	// Service known from parent?
	service = _parent_services.find(service_name);
	if(service)
		return service;

	// custom service?
	if(!Genode::strcmp(service_name, "PD"))
	{
	  return pd_handler->pd_service;
	}
	else if(!Genode::strcmp(service_name, "CPU"))
	{
	  return cpu_handler->cpu_service;
	}
	else if(!Genode::strcmp(service_name, "RAM"))
	{
	  return ram_handler->ram_service;
	}


	// Service not known, cannot intercept it
	if(!service)
	{
		service = new (_md_alloc) Genode::Parent_service(service_name);
		_parent_services.insert(service);
		Genode::warning("Unknown service: ", service_name);
	}

	return service;
}


void Target_child::filter_session_args(const char *service, char *args, Genode::size_t args_len)
{
	Genode::Arg_string::set_arg_string(args, args_len, "label", _name.string());
}
