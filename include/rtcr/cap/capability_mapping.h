/*
 * \brief  Dataspace Handler
 * \author Johannes Fischer
 * \date   2019-08-27
 */

#ifndef _RTCR_CAPABILITY_MAPPING_H_
#define _RTCR_CAPABILITY_MAPPING_H_

/* Genode includes */
#include <foc_native_pd/client.h>

/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/cap/kcap_badge_info.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/rm/region_map_component.h>
#include <rtcr/rm/attached_region_info.h>

namespace Rtcr {
	class Capability_mapping;
}

using namespace Rtcr;


class Rtcr::Capability_mapping : public Checkpointable
{
protected:
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;

  Genode::List<Kcap_badge_info> _kcap_mapping;
  Genode::addr_t _cap_idx_alloc_addr;
  Pd_session_component &_pd_session;
  
  void checkpoint() override;
  
public:
        Capability_mapping(Genode::Env &env,
			   Genode::Allocator &alloc,
			   Pd_session_component &pd_session,
			   Genode::Xml_node *config);


  ~Capability_mapping();

  Genode::addr_t find_kcap_by_badge(Genode::uint16_t badge);
};



#endif /* _RTCR_CAPABILITY_MAPPING_H_ */
