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
	_module(module),
	_pd_session(env),
	_parent_services(parent_services),
	_child_ep (_env, 16*1024, "child ep"),
	_child(_env.rm(), _child_ep.rpc_ep(), *this)
	
{
  DEBUG_THIS_CALL PROFILE_THIS_CALL;
  _pd_session_cap = _pd_session.cap();
  //  Genode::Pd_session *genode_pd_session = &_pd_session;
  //  Pd_session *pd_session = static_cast<Pd_session *>(genode_pd_session);
  //  _pd_session_cap = pd_session->parent_cap();
  Genode::log("Child::parent_pd_cap=", _pd_session_cap);
}


void Child::init(Genode::Pd_session &session, Genode::Capability<Genode::Pd_session> /*cap*/)
{
  DEBUG_THIS_CALL PROFILE_THIS_CALL;

  Genode::Ram_quota quota;
  quota.value=150000;

  Genode::Cap_quota caps;
  caps.value=1000;

  
  session.ref_account(_env.pd_session_cap());
  Pd_session &_session = static_cast<Pd_session &>(session);

  //  try {
    _env.pd().transfer_quota(_session.parent_cap(), caps);
    //  } catch (Genode::Out_of_caps) { }	

    //  try {
    _env.pd().transfer_quota(_session.parent_cap(), quota);
    //  } catch (Genode::Out_of_ram) { }

  Genode::log("_session ram after transfer: ",_session.ram_quota().value);
  Genode::log("_session cap after transfer: ",_session.cap_quota().value);
  Genode::log("session.parent ram after transfer: ",_session._parent_pd.ram_quota().value);
  Genode::log("session.parent cap after transfer: ",_session._parent_pd.cap_quota().value);  
}

void Child::init(Genode::Cpu_session &, Genode::Capability<Genode::Cpu_session>)
{
  DEBUG_THIS_CALL PROFILE_THIS_CALL;  

}


Genode::Pd_session &Child::ref_pd()
{
  DEBUG_THIS_CALL PROFILE_THIS_CALL;    
  return _pd_session;
}


Genode::Pd_session_capability Child::ref_pd_cap() const
{
  DEBUG_THIS_CALL PROFILE_THIS_CALL;
  return _pd_session_cap;
}


Genode::Child_policy::Route Child::resolve_session_request(Genode::Service::Name const &name,
		                              Genode::Session_label const &label)
{
  Genode::Service *service = 0;
  DEBUG_THIS_CALL PROFILE_THIS_CALL;
  Genode::log("resolve_session_request name=",name," label=", label);

  /* service is provided by module */
  if(!Genode::strcmp(_name,label.string())) {
      service = _module.resolve_session_request(name.string(), label.string());
  }

  /* service is provided by parent */
  if(!service){
	_parent_services.for_each([&] (Genode::Registered<Genode::Parent_service> &s) {
	      if (service || s.name() != name)
		return;
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

}


// void Child::filter_session_args(const char *service,
// 									   char *args,
// 									   Genode::size_t args_len)
// {
// #ifdef DEBUG
// 	Genode::log("\033[36m", __PRETTY_FUNCTION__, "\033[0m");
// #endif
// 	Genode::Session_label const old_label = Genode::label_from_args(args);
// 	if (old_label == "") {
// 		Genode::Arg_string::set_arg_string(args, args_len, "label", _name);
// 	} else {
// 		Genode::Session_label const name(_name);
// 		Genode::Session_label const new_label = prefixed_label(name, old_label);
// 		Genode::Arg_string::set_arg_string(args, args_len, "label", new_label.string());
// 	}
// }

