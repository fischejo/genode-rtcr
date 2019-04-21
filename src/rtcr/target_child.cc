/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-08-04
 */

#include <rtcr/target_child.h>
#include <rtcr/core_module_abstract.h>

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
	_parent_services (parent_services),
	core(nullptr)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
	
	/* print all registered session handlers */
        Module_factory* factory = Module_factory::first();
	while(factory) {
	      Genode::log("\e[38;5;214m", "Found Linked Module: \e[1m", factory->name(), "\033[0m");
	      factory = factory->next();
	}    

	/* parse module nodes of config */
	const Genode::Xml_node& config_node = Genode::config()->xml_node();

	/* load modules */
	try {
	    Genode::Xml_node module_node = config_node.sub_node("module");
	    while(true) {
	      /* ignore modules which are manually disabled */
		if (!module_node.attribute_value("disabled", false)) {
		  /* parse module name and provide string */
		  Module_name const name = module_node.attribute_value("name", Module_name());

		    /* find factory for module */
		    Module_factory *factory = Module_factory::get(name);
		    if(!factory)
			Genode::error("Module '", name, "' is not linked!");

		    /* create module */
		    Module *module = factory->create(env,
						     md_alloc,
						     _resources_ep,
						     _name.string(),
						     _in_bootstrap,
						     &module_node);

		    modules.insert(module);
		    Genode::log("\e[38;5;214m", "Module loaded: \e[1m", name, "\033[0m");

		    Core_module_abstract *core_module = dynamic_cast<Core_module_abstract*>(module);
		    if (!core && core_module) {
		        core = core_module;
			Genode::log("\e[38;5;214m", "Module \e[1m" , name, "\e[0m\e[38;5;214m chosen as core module", "\033[0m");
		    }
		    
		    module_node = module_node.next("module");		
		}
	    }
	} catch (Genode::Xml_node::Nonexistent_sub_node n) {
	  Genode::log("\e[38;5;214m", "Module loading finished", "\033[0m");
	}

	/* make sure that there is a module which provides `core`. */
	if(!core)
	    Genode::error("No module found which provides `core`!");


	/* inform every module about each other */
	Module *module = modules.first();
	while (module) {
	  Genode::log("\e[38;5;214m", "module[", "\e[1m", module->name(),  "\e[0m\e[38;5;214m","]->initialize()", "\033[0m");      	  
	  module->initialize(modules);
	  module = module->next();
	}


	_address_space = new(md_alloc) Genode::Region_map_client(core->pd_session().address_space());
	//	Genode::log("\e[1m\e[38;5;199m", "After Region_map_client", "\033[0m");	

	_initial_thread = new(md_alloc) Genode::Child::Initial_thread(
								      core->cpu_session(),
								      core->pd_session().cap(),
								      _name.string());
Genode::log("\e[1m\e[38;5;199m", "After Initial_thread", "\033[0m");	
}


Target_child::~Target_child()
{
	if(_child)
		Genode::destroy(_md_alloc, _child);

	Genode::log("\033[33m", __func__, "\033[0m() ", _name.string());
}


void Target_child::start()
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

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
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
    core->pause();
    // Module *module = modules.first();
    // while (module) {
    //   Genode::log("\e[38;5;214m", "module[", "\e[1m", module->name(),  "\e[0m\e[38;5;214m","]->checkpoint()", "\033[0m");      
    // 	module->checkpoint(state);
    // 	module = module->next();
    // }
    core->resume();
}


void Target_child::restore(Target_state &state)
{
#ifdef DEBUG
    Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
    Module *module = modules.first();
    while (module) {
	module->restore(state);
	module = module->next();
    }    
}


Genode::Service *Target_child::resolve_session_request(const char *service_name,
						       const char *args)
{
#ifdef DEBUG
	Genode::log("\033[36m", __func__,"(",service_name, ", ", args, ")", "\033[0m");
#endif


	if(!Genode::strcmp(service_name, "LOG") && _in_bootstrap) {
		Genode::log("  Unsetting bootstrap_phase");
		_in_bootstrap = false;
	}
	
	// Service known from parent?
	Genode::Service *service = _parent_services.find(service_name);
	if(service) {
	    return service;
	    Genode::log("parent service");
	}

	// ask all modules
	Module *module = modules.first();
	while (module) {
	    service = module->resolve_session_request(service_name, args);
	    if(service) {
	      Genode::log("module service: ", module->name());
	      return service;	    
	    }

	    module = module->next();
	}

	// Service not known, cannot intercept it
	if(!service) {
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
