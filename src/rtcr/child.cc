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
			 Init_module &module)
	:
	_env(env),
	_alloc (alloc),
	_name (name),	
	_module(module),	
	_pd_session(create_pd_session()),
	_pd_session_cap(_pd_session.cap()),
	_pd_factory(_pd_session),
	_pd_service(_pd_factory),
	_parent_services(parent_services),
	_config(env, "config"),
	_caps_quota(read_cap_quota()),
	_ram_quota(read_ram_quota()),
	_child_ep(env, 4*1024*sizeof(Genode::addr_t), "child_ep", Genode::Affinity::Location()),
	_child(env.rm(), _child_ep.rpc_ep(), *this)
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
	Genode::log("read_cap_quota=",caps_quota.value);
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
	Genode::log("read_ram_quota=",ram_quota.value);
	return ram_quota;
}


void Child::init(Genode::Pd_session &session, Genode::Capability<Genode::Pd_session> cap)
{
	DEBUG_THIS_CALL;
	session.ref_account(_env.pd_session_cap());
	_env.pd().transfer_quota(_pd_session.parent_cap(), _caps_quota);
	_env.pd().transfer_quota(_pd_session.parent_cap(), _ram_quota);
}


void Child::init(Genode::Cpu_session &session, Genode::Capability<Genode::Cpu_session> cap)
{
	DEBUG_THIS_CALL;
	Cpu_session *cpu = dynamic_cast<Cpu_session*>(&session);
	static Genode::size_t avail = Cpu_session::quota_lim_upscale(100, 100);
	session.ref_account(_env.cpu_session_cap());
	_env.cpu().transfer_quota(cpu ? cpu->parent_cap() : cap, avail);
}


Genode::Pd_session &Child::ref_pd()
{
	DEBUG_THIS_CALL;
	return _pd_session;
}


Genode::Pd_session_capability Child::ref_pd_cap() const
{
	DEBUG_THIS_CALL;
	return _pd_session_cap;
}


Genode::Child_policy::Route Child::resolve_session_request(Genode::Service::Name const &name,
                                                           Genode::Session_label const &label)
{
	DEBUG_THIS_CALL;

#ifdef VERBOSE
	Genode::log("resolve_session_request name=",name," label=", label);
#endif

	Genode::Service *service = 0;

	/* PD session is created in advance, so just hand over the corresponding
	 * service */
	if (name == "PD") {
		service = &_pd_service;
	}
	
	/* service is provided locally */
	if (name != "ROM") {
		if(!Genode::strcmp(_name,label.string())) {
			_module.services().for_each([&] (Genode::Service &s) {
				                         if (service || s.name() != name) return;
				                         service = &s;
			                         });
		}
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


Pd_session &Child::create_pd_session()
{
	DEBUG_THIS_CALL;
	
	/* Prepare Session Arguments */
	char args_buf[256];
	Genode::snprintf(args_buf,
					 sizeof(args_buf),
					 "virt_space=%d, ram_quota=%d, cap_quota=%d, label=\"%s\"",
					 1, 1024*1024, 13, _name);

	/* create a Pd_session */
	return _module.pd_factory().create(Genode::Session_state::Args(args_buf),
									   Genode::Affinity());
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

	/* transfer ram */
	if (ram.value) {
		_env.pd().transfer_quota(_pd_session.parent_cap(), ram);
	}

	/* transfer caps */
	if (caps.value) {
		_env.pd().transfer_quota(_pd_session.parent_cap(), caps);
	}

	_child.notify_resource_avail();
}

