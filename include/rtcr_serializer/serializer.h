/*
 * \brief  Serializer for a module state
 * \author Johannes Fischer
 * \date   2019-05-16
 */

#ifndef _RTCR_SERIALIZER_H_
#define _RTCR_SERIALIZER_H_

/* Genode includes */
#include <rm_session/connection.h>
#include <region_map/client.h>

/* Rtcr includes */
#include <rtcr/info_structs.h>
#include <rtcr/cpu/cpu_session.h>
#include <rtcr/pd/pd_session.h>
#include <rtcr/ram/ram_session.h>
#include <rtcr/rm/rm_session.h>
#include <rtcr/log/log_session.h>
#include <rtcr/timer/timer_session.h>
#include <rtcr/rom/rom_session.h>
#include <rtcr/cap/capability_mapping.h>
#include <rtcr/child_info.h>

/* Protobuf includes */
#include "rtcr.pb.h"
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <base/printf.h>


namespace Rtcr {
	class Serializer;
}

using namespace Rtcr;


class Rtcr::Serializer
{
protected:

	Genode::Env &_env;
	Genode::Rm_connection _rm_connection;
	Genode::Allocator &_alloc;	
	
	const Genode::size_t _PAGE_SIZE = 4096;
	inline Genode::size_t page_aligned_size(Genode::size_t size);

	Pb::Normal_info *normal_info(Capability_mapping *cm, Normal_info *info);
	Pb::Session_info *session_info(Capability_mapping *cm, Session_info *info);	
	
    void set_pd_session(Capability_mapping *cm,
						Pb::Child_info *tc,
						Child_info *_tc);
	
	void set_ram_session(Capability_mapping *cm,
						 Pb::Child_info *tc,
						 Child_info *_tc);
	
	void set_cpu_session(Capability_mapping *cm,
						 Pb::Child_info *tc,
						 Child_info *_tc);
	
	void set_timer_session(Capability_mapping *cm,
						   Pb::Child_info *tc,
						   Child_info *_tc);
	
	void set_log_session(Capability_mapping *cm,
						 Pb::Child_info *tc,
						 Child_info *_tc);
	
	void set_rm_session(Capability_mapping *cm,
						Pb::Child_info *tc,
						Child_info *_tc);
	
	void set_rom_session(Capability_mapping *cm,
						 Pb::Child_info *tc,
						 Child_info *_tc);

	void add_region_map(Capability_mapping *cm,
						Pb::Rm_session_info *rm_session,
						Region_map_info *info);
	
	void add_attached_region(Capability_mapping *cm,
							 Pb::Region_map_info *region_map,
							 Attached_region_info *info);
	
	void add_ram_dataspace(Capability_mapping *cm,
						   Pb::Ram_session_info *ram_session,
						   Ram_dataspace_info *info);
	
	void add_cpu_thread(Capability_mapping *cm,
						Pb::Cpu_session_info *cpu_session,
						Cpu_thread_info *info);

	void set_address_space(Capability_mapping *cm,
						   Pb::Pd_session_info *pd_session_info,
						   Region_map_info *_info);
	
	void set_stack_area(Capability_mapping *cm,
						Pb::Pd_session_info *pd_session_info,
						Region_map_info *_info);
	
	void set_linker_area(Capability_mapping *cm,
						 Pb::Pd_session_info *pd_session_info,
						 Region_map_info *_info);
	
	void add_signal_source(Capability_mapping *cm,
						   Pb::Pd_session_info *pd_session,
						   Signal_source_info *info);
	
	void add_signal_context(Capability_mapping *cm,
							Pb::Pd_session_info *pd_session,
							Signal_context_info *info);
	
	void add_native_capability(Capability_mapping *cm,
							   Pb::Pd_session_info *pd_session,
							   Native_capability_info *info);

	
	Genode::Ram_dataspace_capability compress(Genode::size_t *dst_size,
											  Genode::Dataspace_capability src_cap,
											  Genode::size_t src_size);
public:

	Serializer(Genode::Env &env, Genode::Allocator &alloc);
	~Serializer() {}

	Genode::Ram_dataspace_capability serialize(Child_info *child, Genode::size_t *size);
	void parse(Genode::Dataspace_capability ds_cap);
};


#endif /* _RTCR_SERIALIZER_H_ */
