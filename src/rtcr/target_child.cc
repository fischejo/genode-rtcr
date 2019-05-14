/*
 * \brief  Child creation
 * \author Denis Huber
 * \date   2016-08-04
 */

#include <rtcr/target_child.h>
#include <rtcr/core_module_abstract.h>

using namespace Rtcr;


Target_child::Target_child(Genode::Env &env,
			   Genode::Allocator &alloc,
			   Genode::Service_registry &parent_services)
	: Target_child(env, alloc, parent_services, _child_name_from_xml()) {}


Child_name Target_child::_child_name_from_xml()
{
	/* parse name of child application from xml config */	
	const Genode::Xml_node& config_node = Genode::config()->xml_node();
	Genode::Xml_node child_node = config_node.sub_node("child");
	Child_name child_name = child_node.attribute_value("name", Child_name());	
	Genode::log("\e[38;5;214m", "Use XML defined child name: \e[1m", child_name, "\033[0m");
	return child_name;
}


Target_child::Target_child(Genode::Env &env,
			   Genode::Allocator &alloc,
			   Genode::Service_registry &parent_services,
			   Child_name name)
	:
	_name            (name),
	_env             (env),
	_alloc        (alloc),
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

	/* get xml node object of configuration */
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
								 alloc,
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


	_address_space = new(alloc) Genode::Region_map_client(core->pd_session().address_space());
	//	Genode::log("\e[1m\e[38;5;199m", "After Region_map_client", "\033[0m");	

	_initial_thread = new(alloc) Genode::Child::Initial_thread(
		core->cpu_session(),
		core->pd_session().cap(),
		_name.string());
	Genode::log("\e[1m\e[38;5;199m", "After Initial_thread", "\033[0m");	
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

	_child = new (_alloc) Genode::Child ( core->rom_connection().dataspace(),
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


void Target_child::checkpoint(Target_state &target_state, bool resume)
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif

	/* pause child */
	core->pause();

	/* checkpoint every module */
	Module *module = modules.first();
	while (module) {
		Genode::log("\e[38;5;214m", "module[", "\e[1m", module->name(),  "\e[0m\e[38;5;214m","]->checkpoint()", "\033[0m");      
		Module_state *module_state = module->checkpoint();

		/* store current module state in the target state */
		if(module_state)
			target_state.store(module->name(), *module_state);
		module = module->next();
	}

	/* resume child */
	if(resume)
		core->resume();
}


void Target_child::restore(Target_state &target_state)
{
#ifdef DEBUG
	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
#endif
  
	Module *module = modules.first();
	while (module) {
		/* lookup if target_state stores a state of the module */
		Module_state *module_state = target_state.state(module->name());

		/* restore module with this state */
		module->restore(module_state);

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
		service = new (_alloc) Genode::Parent_service(service_name);
		_parent_services.insert(service);
		Genode::warning("Unknown service: ", service_name);
	}

	return service;
}


void Target_child::filter_session_args(const char *service, char *args, Genode::size_t args_len)
{
	Genode::Arg_string::set_arg_string(args, args_len, "label", _name.string());
}
