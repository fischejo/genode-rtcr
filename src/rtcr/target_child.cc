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
	Module_factory::print();

	// parse module nodes of config
	const Genode::Xml_node& config_node = Genode::config()->xml_node();

	// load modules
	try {
	    Genode::Xml_node module_node = config_node.sub_node("module");
	    while(true) {
		// ignore modules which are manually disabled
		if (!module_node.attribute_value("disabled", false)) {
		    // parse module name and provide string
		    Module_name const name = module_node.attribute_value("name", Module_name());
		    Module_name const provides = module_node.attribute_value("provides", name);
		    Genode::log("config::module name=", name, " provides=", provides);

		    // find factory for module
		    Module_factory *factory = Module_factory::get(name);
		    if(!factory)
			Genode::error("Module '", name, "' configured but no Module_factory found!");

		    // create module 
		    Module *module = factory->create(env,
						     md_alloc,
						     _resources_ep,
						     _name.string(),
						     _in_bootstrap,
						     module_node);

		    if (!Genode::strcmp(provides.string(), "core")) {
			Genode::log("Found module which provides 'core'.");
			core = dynamic_cast<Core_module*>(module);
		    }

		    modules.insert(module);
		    Genode::log("Module '", name, "' loaded");
		    module_node = module_node.next("module");		
		}
	    }
	} catch (Genode::Xml_node::Nonexistent_sub_node n) {
	    Genode::log("Module loading finished");
	}

	// make sure that there is a module which provides `core`.
	if(!core)
	    Genode::error("No module found which provides `core`!");


        // Donate ram quota to child
	// TODO Replace static quota donation with the amount of quota, the child needs
	Genode::size_t donate_quota = 1024*1024;
	core->ram_session().ref_account(env.ram_session_cap());
	// Note: transfer goes directly to parent's ram session
	env.ram().transfer_quota(core->ram_session().parent_cap(), donate_quota);
	
	// do some magic
	_address_space = new(md_alloc) Genode::Region_map_client(core->pd_session().address_space());


	_initial_thread = new(md_alloc) Genode::Child::Initial_thread(
								      core->cpu_session(),
								      core->pd_session().cap(),
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

	_child = new (_md_alloc) Genode::Child ( core->rom_connection().dataspace(),
						 Genode::Dataspace_capability(),
						 core->pd_session().cap(),
						 core->pd_session(),
						 core->ram_session().cap(),
						 core->ram_session(),
						 core->cpu_session().cap(),
						 *_initial_thread,
						 _env.rm(),
						 *_address_space,
						 _child_ep.rpc_ep(),
						 *this,
						 core->pd_service(),
						 core->ram_service(),
						 core->cpu_service());
}


void Target_child::checkpoint(Target_state &state)
{
    Module *module = modules.first();
    while (module) {
	module->checkpoint(state);
	module = module->next();
    }
}


void Target_child::restore(Target_state &state)
{
    Module *module = modules.first();
    while (module) {
	module->restore(state);
	module->next();
    }    
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
	  return &core->pd_service();
	}
	else if(!Genode::strcmp(service_name, "CPU"))
	{
	  return &core->cpu_service();
	}
	else if(!Genode::strcmp(service_name, "RAM"))
	{
	  return &core->ram_service();
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
