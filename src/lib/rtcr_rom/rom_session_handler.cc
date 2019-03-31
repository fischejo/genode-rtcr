/*
 * \brief  RAM Session Handler
 * \author Johannes Fischer
 * \date   2019-03-21
 */

#include <rtcr_rom/rom_session_handler.h>


using namespace Rtcr;

// This line registers the Cpu_session_handler during application start.
Rtcr::Rom_session_handler_factory _rom_session_handler_factory;


Rom_session_handler::Rom_session_handler(Genode::Env &env,
					Genode::Allocator &md_alloc,
					Genode::Entrypoint &ep,
					const char* label,
					bool &bootstrap)
  :
  _env(env),
  _md_alloc(md_alloc),
  _ep(ep)
{
	rom_root = new (_md_alloc) Rom_root(_env,
					    _md_alloc,
					    _ep,
					    bootstrap);
	
	rom_service = new (_md_alloc) Genode::Local_service("ROM", rom_root);

	rom_connection = new (_md_alloc) Genode::Rom_connection(_env, label);
}

Rom_session_handler::~Rom_session_handler()
{
  if(rom_root)    Genode::destroy(_md_alloc, rom_root);
  if(rom_service) Genode::destroy(_md_alloc, rom_service);
}


void Rom_session_handler::session_init()
{
  Genode::log("Rom_session_handler::session_init()");
}
void Rom_session_handler::session_checkpoint()
{
  Genode::log("Rom_session_handler::session_checkpoint()");
}
void Rom_session_handler::session_restore()
{
  Genode::log("Rom_session_handler::session_restore()");
}


