/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_rm/rm_session_handler.h>


using namespace Rtcr;

// This line registers the Cpu_session_handler during application start.
Rtcr::Rm_session_handler_factory _rm_session_handler_factory;


Rm_session_handler::Rm_session_handler(Genode::Env &env,
					Genode::Allocator &md_alloc,
					Genode::Entrypoint &ep,
					const char* label,
					bool &bootstrap)
  :
  _env(env),
  _md_alloc(md_alloc),
  _ep(ep)
{
	rm_root = new (_md_alloc) Rm_root(_env,
					    _md_alloc,
					    _ep,
					    bootstrap);
	
	rm_service = new (_md_alloc) Genode::Local_service("RM", rm_root);

}  

Rm_session_handler::~Rm_session_handler()
{
  if(rm_root)    Genode::destroy(_md_alloc, rm_root);
  if(rm_service) Genode::destroy(_md_alloc, rm_service);
}




void Rm_session_handler::session_init()
{
  Genode::log("Rm_session_handler::session_init()");
}
void Rm_session_handler::session_checkpoint()
{
  Genode::log("Rm_session_handler::session_checkpoint()");
}
void Rm_session_handler::session_restore()
{
  Genode::log("Rm_session_handler::session_restore()");
}


