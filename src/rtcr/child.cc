/*
 * \brief  Child creation
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/child.h>

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


Child::Child(Genode::Env &env,
             Genode::Allocator &alloc,
             const char* name,
             Genode::Registry<Genode::Registered<Genode::Parent_service>> &parent_services,
             Base_module &module)
	:
	_name (name),
	_env (env),
	_alloc (alloc),
	_pd_session(env),
	_parent_services(parent_services),
	_module(module),
	_config(_env, "config"),
	_caps_quota(read_cap_quota()),
	_ram_quota(read_ram_quota()),	
	_child_ep (_env, 16*1024, "child ep"),
	_child(_env.rm(), _child_ep.rpc_ep(), *this)
{
	DEBUG_THIS_CALL;
}


Genode::Cap_quota Child::read_cap_quota()
{
	Genode::Xml_node config_node = _config.xml();
	Genode::Xml_node child_node = config_node.sub_node("child");
	Genode::String<30> child_name;
	bool found = false;
	Genode::Cap_quota caps_quota;

	/* find child node in XML config */
	while(!found) {
		child_name = child_node.attribute_value("name", child_name);
		if(!Genode::strcmp(_name, child_name.string())) {
			found = true;
		} else if(child_node.last()) {
			break;
		} else {
			child_node = child_node.next("child");
		}
	}

	if(found) {
		if(!child_node.has_attribute("caps")) {
			Genode::error("Child ",_name, " has no cap quota defined");
			throw Genode::Exception();
		}
		caps_quota.value = child_node.attribute_value<long>("caps", 0);
	} else {
		Genode::error("Child config not found.");
		throw Genode::Exception();
	}

	return caps_quota;
}


Genode::Ram_quota Child::read_ram_quota()
{
	Genode::Xml_node config_node = _config.xml();
	Genode::Xml_node child_node = config_node.sub_node("child");
	Genode::String<30> child_name;
	bool found = false;
	Genode::Ram_quota ram_quota;

	/* find child node in XML config */
	while(!found) {
		child_name = child_node.attribute_value("name", child_name);
		if(!Genode::strcmp(_name, child_name.string())) {
			found = true;
		} else if(child_node.last()) {
			break;
		} else {
			child_node = child_node.next("child");
		}
	}

	if(found) {
		if(!child_node.has_attribute("caps")) {
			Genode::error("Child ",_name, " has no ram quota defined");
			throw Genode::Exception();
		}
		ram_quota.value = child_node.attribute_value<long>("quota", 0);
	} else {
		Genode::error("Child config not found.");
		throw Genode::Exception();
	}

	return ram_quota;
}


void Child::init(Genode::Pd_session &session, Genode::Capability<Genode::Pd_session> /*cap*/)
{
	DEBUG_THIS_CALL;
	session.ref_account(_env.pd_session_cap());

	Pd_session &_session = static_cast<Pd_session &>(session);
	_parent_pd_cap = _session.parent_cap();
	_env.pd().transfer_quota(_parent_pd_cap, _caps_quota);
	_env.pd().transfer_quota(_parent_pd_cap, _ram_quota);

#ifdef VERBOSE
	Genode::log("Pd_session[",_name,"]",
	            " ram_quota=",_session.ram_quota().value,
	            " cap_quota=",_session.cap_quota().value);
#endif
}


void Child::init(Genode::Cpu_session &, Genode::Capability<Genode::Cpu_session>)
{
	DEBUG_THIS_CALL;
}


Genode::Pd_session &Child::ref_pd()
{
	DEBUG_THIS_CALL;
	return _pd_session;
}


Genode::Pd_session_capability Child::ref_pd_cap() const
{
	DEBUG_THIS_CALL;
	return _pd_session.cap();
}


Genode::Child_policy::Route Child::resolve_session_request(Genode::Service::Name const &name,
                                                           Genode::Session_label const &label)
{
	DEBUG_THIS_CALL;

#ifdef VERBOSE
	Genode::log("resolve_session_request name=",name," label=", label);
#endif

	Genode::Service *service = 0;

	/* service is provided by module */
	if(!Genode::strcmp(_name,label.string())) {
		service = _module.resolve_session_request(name.string(), label.string());
	}

	/* service is provided by parent */
	if(!service){
		_parent_services.for_each([&] (Genode::Registered<Genode::Parent_service> &s) {
			                          if (service || s.name() != name) return;
			                          service = &s;
		                          });
	}

	if(!service) {
		Genode::warning("Unknown service: ", name);
	}

	return Route { *service, label, Genode::Session::Diag{false} };
}


void Child::resource_request(Genode::Parent::Resource_args const &args)
{
	DEBUG_THIS_CALL;

	Genode::Ram_quota ram = Genode::ram_quota_from_args(args.string());
	Genode::Cap_quota caps = Genode::cap_quota_from_args(args.string());

#ifdef VERBOSE
	Genode::log("requested: ",ram.value," ",caps.value," available: ",
	            _env.pd().avail_ram().value," ",_env.pd().avail_caps().value);
#endif
	ram.value *= 5;
	
	if (ram.value) {
		Genode::Ram_quota avail = _env.pd().avail_ram();
		if (avail.value > ram.value) {
			try {
				_env.pd().transfer_quota(_parent_pd_cap, ram);
			} catch (Genode::Out_of_ram) {
				Genode::log("Something went wrong during RAM transfer");
			}	
		} else {
			try {
				_env.pd().transfer_quota(_parent_pd_cap, Genode::Ram_quota{avail.value >> 1});
			} catch (Genode::Out_of_ram) {
				Genode::log("Something went wrong during RAM transfer");				
			}	
			_env.parent().resource_request(args);
		}
	}

	if (caps.value) {
		Genode::Cap_quota avail = _env.pd().avail_caps();
		if (avail.value > caps.value) {
			try {
				_env.pd().transfer_quota(_parent_pd_cap, caps);
			} catch (Genode::Out_of_caps) {
				Genode::log("Something went wrong during cap transfer");
			}	
		} else {
			try {
				_env.pd().transfer_quota(_parent_pd_cap, Genode::Cap_quota{avail.value >> 1});
			} catch (Genode::Out_of_caps) {
				Genode::log("Something went wrong during cap transfer");
			}	
			_env.parent().resource_request(args);
		}
	}
	_child.notify_resource_avail();	
}
