/*
 * \brief  Thead for checkpointing all capabilities
 * \author Johannes Fischer
 * \date   2019-08-29
 */

#ifndef _RTCR_CAPABILITY_MAPPING_H_
#define _RTCR_CAPABILITY_MAPPING_H_


/* Rtcr includes */
#include <rtcr/checkpointable.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/rm/region_map.h>
#include <rtcr/rm/attached_region.h>
#include <rtcr/rm/attached_region_info.h>
#include <rtcr/rm/region_map_info.h>

namespace Rtcr {
	class Pd_session;
	class Capability_mapping;
}

using namespace Rtcr;


class Rtcr::Capability_mapping : public Checkpointable
{
protected:
	struct Kcap_badge {
		Genode::addr_t   kcap;
		Genode::uint16_t badge;
	} _kcap_mapping[4096];
	
	Genode::size_t index;
	
	Genode::Env        &_env;
	Genode::Allocator  &_alloc;

	Genode::addr_t _cap_idx_alloc_addr;

	/* PD session from which the capabilties are extracted */
	Pd_session *_pd_session;
  
	void checkpoint() override;
  
public:
	Capability_mapping(Genode::Env &env,
	                   Genode::Allocator &alloc,
	                   Pd_session *pd_session);

	~Capability_mapping();

	void print(Genode::Output &output) const;

	
	/**
	 * Method for finiding a capabilitiy based on its badge 
	 *
	 * \param badge
	 */
	Genode::addr_t find_kcap_by_badge(Genode::uint16_t badge);
};



#endif /* _RTCR_CAPABILITY_MAPPING_H_ */
