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
	
	struct Attachment : Genode::List<Attachment>::Element {
		Pb::Attachment *pb;
		Genode::size_t size;
		void *addr;
		Genode::Dataspace_capability cap;
		
		Attachment(Genode::Dataspace_capability _cap, Genode::size_t _size, Pb::Attachment *_pb)
			: pb(_pb), size(_size), cap(_cap) {};
		Attachment(Genode::Dataspace_capability _cap, Genode::size_t _size)
			: pb(nullptr), size(_size), cap(_cap) {};

		Attachment() {};
	};

	struct Rom_attachment : Attachment {
		Genode::Rom_connection rom;
		const Genode::Dataspace_capability rom_cap;
		Genode::Dataspace_client rom_client;

		Rom_attachment(Genode::Env &env, const char *rom_name, Pb::Attachment *_pb)
			:
			rom(env, rom_name),
			rom_cap(rom.dataspace()),
			rom_client(rom_cap)
			{
				pb = _pb;
				size = rom_client.size();
				cap = rom_cap;
			};
	};
	
	/**
	 * General stuff 
	 */
	Genode::Env &_env;
	Genode::Rm_connection _rm_connection;
	Genode::Allocator &_alloc;	
	
	const Genode::size_t _PAGE_SIZE = 4096;
	inline Genode::size_t page_aligned_size(Genode::size_t size);

	/**
	 * compressing and serializing
	 */

	void free(Genode::List<Attachment> &as);	
	void detach(Genode::Region_map_client &rm, Genode::List<Attachment> &as);
	void attach(Genode::Region_map_client &rm, Genode::List<Attachment> &as);	
	Genode::size_t size(Genode::List<Attachment> &as);
	
	Genode::Ram_dataspace_capability compress(Genode::Dataspace_capability src_cap,
										  Genode::size_t src_size,
										  Genode::size_t *dst_size);
	
	void add_child_info(Pb::Child_list *ts,
						Child_info *_tc,
						bool include_binary,
						Genode::List<Attachment> &as);
	
    void set_binary_info(Pb::Child_info *tc,
						 Child_info *_tc,
						 Genode::List<Attachment> &as);
	
	Pb::Normal_info *normal_info(Capability_mapping *cm, Normal_info *info);
	Pb::Session_info *session_info(Capability_mapping *cm, Session_info *info);	
	
    void set_pd_session(Capability_mapping *cm,
						Pb::Child_info *tc,
						Child_info *_tc);
	
	void set_ram_session(Capability_mapping *cm,
						 Pb::Child_info *tc,
						 Child_info *_tc,
						 Genode::List<Attachment> &as);
	
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
						   Ram_dataspace_info *info,
						   Genode::List<Attachment> &as);
	
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


	/**
	 * uncompressing and deserializing
	 */
	Child_info *parse_child_info(const Pb::Child_info &child, void *raw_addr);

	void parse_session_info(const Pb::Session_info &info, Session_info *_info);
	void parse_normal_info(const Pb::Normal_info &info, Normal_info *_info);		
	
	Pd_session_info *parse_pd_session(const Pb::Pd_session_info &info);
	Cpu_session_info *parse_cpu_session(const Pb::Cpu_session_info &info);
	Ram_session_info *parse_ram_session(const Pb::Ram_session_info &info, void *raw_addr);
	Rm_session_info *parse_rm_session(const Pb::Rm_session_info &info);
	Log_session_info *parse_log_session(const Pb::Log_session_info &info);
	Timer_session_info *parse_timer_session(const Pb::Timer_session_info &info);
	Rom_session_info *parse_rom_session(const Pb::Rom_session_info &info);

	
	Signal_source_info *parse_signal_source(const Pb::Signal_source_info &info);
	Signal_context_info *parse_signal_context(const Pb::Signal_context_info &info);
	Native_capability_info *parse_native_capability(const Pb::Native_capability_info &info);
	Cpu_thread_info *parse_cpu_thread(const Pb::Cpu_thread_info &info);
	Ram_dataspace_info *parse_ram_dataspace(const Pb::Ram_dataspace_info &info,
											void *raw_addr);
	Region_map_info *parse_region_map(const Pb::Region_map_info &info);
	Attached_region_info *parse_attached_region(const Pb::Attached_region_info &info);

public:

	Serializer(Genode::Env &env, Genode::Allocator &alloc);
	~Serializer() {}


	Genode::List<Child_info> *parse(Genode::Dataspace_capability ds_cap);


	Genode::Ram_dataspace_capability serialize(Genode::List<Child_info> *_child_list,
											   Genode::size_t *compressed_size,
											   bool include_binary = false);
};


#endif /* _RTCR_SERIALIZER_H_ */
