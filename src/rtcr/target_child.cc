/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-08-04
 */

#include <rtcr/target_child.h>

#include <base/rpc_server.h>
#include <base/session_label.h>
#include <util/arg_string.h>
#include <base/session_label.h>

using namespace Rtcr;


Target_child::Target_child(Genode::Env &env,
			   Genode::Allocator &alloc,
			   Genode::Service_registry &parent_services)
	: Target_child(env,
		       alloc,
		       parent_services,
		       _read_name()) {}


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
	/* parse name of child application from xml config */	
	const Genode::Xml_node& config_node = Genode::config()->xml_node();
	Genode::Xml_node module_node = config_node.sub_node("module");
	Module_name module_name = module_node.attribute_value("name", Module_name());
	return module_name;
}


Genode::Affinity::Location Target_child::_read_affinity_location()
{
	try {	
		/* parse name of child application from xml config */	
		const Genode::Xml_node& config_node = Genode::config()->xml_node();
		Genode::Xml_node child_node = config_node.sub_node("child");
		Genode::Xml_node affinity_node = child_node.sub_node("affinity");

		long const xpos = affinity_node.attribute_value<long>("xpos", 0);
		long const ypos = affinity_node.attribute_value<long>("ypos", 0);
		return Genode::Affinity::Location(xpos, ypos, 1 ,1);
	}
	catch (...) { return Genode::Affinity::Location(0, 0, 1, 1);}
}


Module &Target_child::_load_module(Module_name name)
{
  /* find factory for module */
  Module_factory *factory = Module_factory::get(name);
  if(!factory) {
    Genode::error("Module '", name, "' is not linked!");
  } else {
    /* create module */
    Module *module = factory->create(_env,
				     _alloc,
				     _resources_ep,
				     _name.string(),
				     _in_bootstrap,
				     nullptr);

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
	_affinity_location(_read_affinity_location()),
	_entrypoint(&_cap_session,
		    ENTRYPOINT_STACK_SIZE,
		    "dreikäsehoch",
		    false,
		    _affinity_location),
	_in_bootstrap    (true),
	_parent_services (parent_services),
	_module(_load_module(_read_module_name()))
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

	_address_space = new(alloc) Genode::Region_map_client(
		_module.pd_session().address_space());

	_initial_thread = new(alloc) Genode::Child::Initial_thread(
		_module.cpu_session(),
		_module.pd_session().cap(),
		_name.string());

	_child = new (_alloc) Genode::Child ( _module.rom_connection().dataspace(),
					      Genode::Dataspace_capability(),
					      _module.pd_session().cap(),
					      _module.pd_session(),
					      _module.ram_session().cap(),
					      _module.ram_session(),
					      _module.cpu_session().cap(),
					      *_initial_thread,
					      _env.rm(),
					      *_address_space,
					      _entrypoint,
					      *this,
					      _module.pd_service(),
					      _module.ram_service(),
					      _module.cpu_service());
}


Target_child::~Target_child()
{
	if(_child) Genode::destroy(_alloc, _child);
	Genode::destroy(_alloc, _address_space);
	Genode::destroy(_alloc, _initial_thread);
}


void Target_child::start()
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
	_entrypoint.activate();
}


void Target_child::checkpoint(bool resume)
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
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
