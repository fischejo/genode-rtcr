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
						   Genode::Service_registry &parent_services)
	: Target_child(env,
				   alloc,
				   parent_services,
				   _read_name())
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
}


Child_name Target_child::_read_name()
{
	/* parse name of child application from xml config */	
	const Genode::Xml_node& config_node = Genode::config()->xml_node();
	Genode::Xml_node child_node = config_node.sub_node("child");
	Child_name child_name = child_node.attribute_value("name", Child_name());	
	Genode::log("\e[38;5;214m", "Use XML defined child name: \e[1m", child_name, "\033[0m");
	return child_name;
}


Module_name Target_child::_read_module_name()
{
	DEBUG_THIS_CALL
	/* parse name of child application from xml config */	
	const Genode::Xml_node& config_node = Genode::config()->xml_node();
	Genode::Xml_node module_node = config_node.sub_node("module");
	Module_name module_name = module_node.attribute_value("name", Module_name());
	return module_name;
}


Module &Target_child::_load_module(Module_name name)
{
	DEBUG_THIS_CALL

	/* find factory for module */
	Module_factory *factory = Module_factory::get(name);

	Genode::Xml_node config_node = Genode::config()->xml_node();
	if(!factory) {
		Genode::error("Module '", name, "' is not linked!");
	} else {
		/* create module */
		Module *module = factory->create(_env,
										 _alloc,
										 _resources_ep,
										 _name.string(),
										 _in_bootstrap,
									     &config_node);

		Genode::log("\e[38;5;214m", "Module loaded: \e[1m", name, "\033[0m");    
		return *module;
	}
}



Target_child::Target_child(Genode::Env &env,
						   Genode::Allocator &alloc,
						   Genode::Service_registry &parent_services,
						   Child_name name)
	:
	_name (name),
	_env (env),
	_alloc (alloc),
	_resources_ep (_env, 16*1024, "resources ep"),
	_entrypoint(&_cap_session,
				ENTRYPOINT_STACK_SIZE,
				"entrypoint",
				false,
				_affinity_location), /* TODO: FJO still necessary? */
	_in_bootstrap    (true),
	_parent_services (parent_services),
	_module(_load_module(_read_module_name())),
	_address_space(_module.pd_session().address_space()),
	_initial_thread(_module.cpu_session(),
					_module.pd_session().cap(),
					_name.string()),
	_child(_module.rom_connection().dataspace(),
		   Genode::Dataspace_capability(),
		   _module.pd_session().cap(),
		   _module.pd_session(),
		   _module.ram_session().cap(),
		   _module.ram_session(),
		   _module.cpu_session().cap(),
		   _initial_thread,
		   _env.rm(),
		   _address_space,
		   _entrypoint,
		   *this,
		   _module.pd_service(),
		   _module.ram_service(),
		   _module.cpu_service())

{
	DEBUG_THIS_CALL PROFILE_THIS_CALL	

}


Target_child::~Target_child()
{
//	if(_child) Genode::destroy(_alloc, _child);
//	Genode::destroy(_alloc, _address_space);
//	Genode::destroy(_alloc, _initial_thread);
}


void Target_child::start()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL		

	_entrypoint.activate();
}


void Target_child::checkpoint(bool resume)
{
	_module.checkpoint(resume);
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

	service = _module.resolve_session_request(service_name, args);
	if(service) {
		return service;	    
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
