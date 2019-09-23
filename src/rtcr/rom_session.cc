/*
 * \brief  Intercepting ROM session
 * \author Denis Huber
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#include <rtcr/rom/rom_session.h>
#include <base/session_label.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("lime");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;48m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif


using namespace Rtcr;


Rtcr::Rom_session::Rom_session(Genode::Env& env,
                               Genode::Allocator& md_alloc,
                               Genode::Entrypoint& ep,
                               const char *creation_args,
                               Child_info *child_info)
	:
	Checkpointable(env, "rom_session"),
	Rom_session_info(creation_args, cap().local_name()),
	_env          (env),
	_md_alloc     (md_alloc),
	_ep           (ep),
	_parent_rom   (env, Genode::label_from_args(creation_args).string()),
	_child_info (child_info)

{
	DEBUG_THIS_CALL;
	_ep.rpc_ep().manage(this);
	child_info->rom_session = this;	
}


Rom_session::~Rom_session() {
	_ep.rpc_ep().dissolve(this);
	_child_info->rom_session = nullptr;	
}


void Rom_session::checkpoint()
{
	DEBUG_THIS_CALL PROFILE_THIS_CALL
		i_upgrade_args = _upgrade_args;
	i_dataspace_badge = _dataspace.local_name();
	i_sigh_badge = _sigh.local_name();
}


Genode::Rom_dataspace_capability Rtcr::Rom_session::dataspace()
{
	auto result = _parent_rom.dataspace();
	_dataspace = result;
	_size = Genode::Dataspace_client(Genode::static_cap_cast<Genode::Dataspace>(result)).size();
	return result;
}


bool Rtcr::Rom_session::update()
{
	return _parent_rom.update();
}


void Rtcr::Rom_session::sigh(Genode::Signal_context_capability sigh)
{
	_sigh = sigh;
	_parent_rom.sigh(sigh);
}


void Rom_session::upgrade(const char *upgrade_args)
{
	/* instead of upgrading the intercepting session, the
	   intercepted session is upgraded */
	_env.parent().upgrade(Genode::Parent::Env::pd(), upgrade_args);
	_upgrade_args = upgrade_args;
}

