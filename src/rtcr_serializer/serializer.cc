/*
 * \brief  Serializer
 * \author Johannes Fischer
 * \date   2019-08-31
 */

#include <rtcr_serializer/serializer.h>
#include "zlib.h"
#include <base/fixed_stdint.h>

#ifdef PROFILE
#include <util/profiler.h>
#define PROFILE_THIS_CALL PROFILE_FUNCTION("violet");
#else
#define PROFILE_THIS_CALL
#endif

#if DEBUG 
#define DEBUG_THIS_CALL Genode::log("\e[38;5;207m", __PRETTY_FUNCTION__, "\033[0m");
#else
#define DEBUG_THIS_CALL
#endif

using namespace Rtcr;

Serializer::Serializer(Genode::Env &env, Genode::Allocator &alloc)
	: _env(env), _rm_connection(env), _alloc(alloc) {}


Genode::Ram_dataspace_capability Serializer::serialize(Child_info *_child_info,
													   Genode::size_t *_compressed_size)
{
	DEBUG_THIS_CALL; PROFILE_THIS_CALL;
	
	/* convert session information to protobuf objects */
	Pb::Child_info *child_info = new(_alloc) Pb::Child_info();
	Capability_mapping *cm = _child_info->capability_mapping;
	set_pd_session(cm, child_info, _child_info);
	set_ram_session(cm, child_info, _child_info);
	set_cpu_session(cm, child_info, _child_info);
	set_timer_session(cm, child_info, _child_info);
	set_log_session(cm, child_info, _child_info);
	set_rm_session(cm, child_info, _child_info);
	set_rom_session(cm, child_info, _child_info);

	/* serialize protobuf objects into a dataspace (pb_ds_cap) */
	Genode::size_t pb_size = child_info->ByteSize();
	Genode::size_t pb_offset = sizeof(Genode::uint32_t);
	Genode::size_t pb_ds_size = page_aligned_size(pb_size + pb_offset);
	Genode::Ram_dataspace_capability pb_ds_cap = _env.ram().alloc(pb_ds_size);	

	/* create region map which will hold all dataspaces */
	Genode::size_t total_size = pb_ds_size;
	Ram_dataspace *ram_dataspace = _child_info->ram_session->info.ram_dataspaces;
	while(ram_dataspace) {
		total_size += page_aligned_size(ram_dataspace->info.size);
		ram_dataspace = ram_dataspace->next();
	}
	Genode::Region_map_client region_map(_rm_connection.create(total_size));

	/* attach protobuf dataspace to region map at position 0 */
	region_map.attach_at(pb_ds_cap, 0);
	Genode::log("Protobuf Dataspace", " local_offset=0x0",
				" ds_size=",Genode::Hex(pb_ds_size),
				" pb_size=", pb_size);

	/* attach all other dataspaces to region map. The local position of each
	 * dataspace in the region is directly stored to the corresponding protobuf
	 * dataspace object. By parsing the protobuf object during deserializion,
	 * the dataspaces can be extracted again. */
	ram_dataspace = _child_info->ram_session->info.ram_dataspaces;
	Pb::Ram_session_info *ram_session_info=child_info->mutable_ram_session_info();
	int i = 0;
	Genode::addr_t local_addr = pb_ds_size;
	while(ram_dataspace) {
		region_map.attach_at(ram_dataspace->info.dst_cap, local_addr);
		Pb::Ram_dataspace_info *ram_dataspace_info = ram_session_info->
			mutable_ram_dataspace_info(i);
		
 		Genode::log("Child Dataspace[",i,"]",
					" local_offset=", Genode::Hex(local_addr),
					" size=",Genode::Hex(ram_dataspace->info.size));

		ram_dataspace_info->set_local_offset(local_addr);

		local_addr += page_aligned_size(ram_dataspace->info.size);
		ram_dataspace = ram_dataspace->next();
		i++;
	}

	void *pb_array = _env.rm().attach(pb_ds_cap);
	*((Genode::uint32_t*) pb_array) = pb_size; 
	child_info->SerializeToArray(pb_array + pb_offset, pb_ds_size);
	_env.rm().detach(pb_array);
	
	/* compress whole region map */
	Genode::size_t estimated_ds_size = compressBound(total_size);
	Genode::size_t compressed_offset = sizeof(Genode::uint32_t);
	Genode::size_t compressed_ds_size = page_aligned_size(
		estimated_ds_size + compressed_offset);
	Genode::Ram_dataspace_capability compressed_ds_cap(
		_env.ram().alloc(compressed_ds_size));
	
	Bytef *dst = _env.rm().attach(compressed_ds_cap);
	Bytef *src = _env.rm().attach(region_map.dataspace());
	uLongf compressed_size = estimated_ds_size;
	int return_value = compress2(dst + compressed_offset*2,
								 &compressed_size,
								 src,
								 total_size,
								 Z_BEST_COMPRESSION);
	*((Genode::uint32_t*) dst) = compressed_size;
	*((Genode::uint32_t*) dst+1) = total_size;
	_env.rm().detach(dst);
	_env.rm().detach(src);

	/* detach all dataspaces from the local region map, otherwise destroying the
	 * region map results in a hang */
	region_map.detach(0); /* pb_ds_cap is attached at offset 0 */
	ram_session_info=child_info->mutable_ram_session_info();	
	for(int n = 0; n < ram_session_info->ram_dataspace_info_size(); n++) {
		Pb::Ram_dataspace_info *info = ram_session_info->mutable_ram_dataspace_info(n);
		Genode::addr_t local_offset = info->local_offset();
		region_map.detach(local_offset);
	}

	_env.ram().free(pb_ds_cap);
// 	_rm_connection.destroy(region_map);  /* hangs... */

	if(return_value != Z_OK) {
		Genode::error("Compression failed");
		throw Genode::Exception();
	}

	Genode::log("compressed_ds_size=", Genode::Hex(compressed_ds_size),
				" rate=",100-(int) (((float)compressed_size/total_size)*100),"%",
				" compressed_size=", Genode::Hex(compressed_size),
				" uncompressed_size=", Genode::Hex(total_size),
				" pb_size=",Genode::Hex(pb_size));

	*_compressed_size = compressed_size;
	return compressed_ds_cap;
}


Genode::size_t Serializer::page_aligned_size(Genode::size_t size)
{
	Genode::size_t _size = (size/_PAGE_SIZE)*_PAGE_SIZE;
	return _size + ((size - _size) ? _PAGE_SIZE : 0);
}


Pb::Normal_info *Serializer::normal_info(Capability_mapping *_cm, Normal_info *_info)
{
	Pb::Normal_info *info = new(_alloc) Pb::Normal_info();
	info->set_kcap(_cm->find_kcap_by_badge(_info->badge));
	info->set_badge(_info->badge);
	info->set_bootstrapped(_info->bootstrapped);
	return info;
}


Pb::Session_info *Serializer::session_info(Capability_mapping *_cm, Session_info *_info)
{
	Pb::Session_info *info = new(_alloc) Pb::Session_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_creation_args(_info->creation_args.string());
	info->set_upgrade_args(_info->upgrade_args.string());
	return info;
}


void Serializer::set_pd_session(Capability_mapping *_cm,
								Pb::Child_info *tc,
								Child_info *_tc)
{
	DEBUG_THIS_CALL;
	Pd_session_info *_info = &_tc->pd_session->info;
	
	Pb::Pd_session_info *info = new(_alloc) Pb::Pd_session_info();
	info->set_allocated_session_info(session_info(_cm, _info));

	set_address_space(_cm, info, &_info->address_space_info);
	set_linker_area(_cm, info, &_info->linker_area_info);
	set_stack_area(_cm, info, &_info->stack_area_info);

	Signal_source *signal_source = _info->signal_sources;
	while(signal_source) {
		add_signal_source(_cm, info, &signal_source->info);
		signal_source = signal_source->next();
	}

	Signal_context *signal_context = _info->signal_contexts;
	while(signal_context) {
		add_signal_context(_cm, info, &signal_context->info);
		signal_context = signal_context->next();
	}

	Native_capability *native_cap = _info->native_caps;
	while(native_cap) {
		add_native_capability(_cm, info, &native_cap->info);
		native_cap = native_cap->next();
	}
	
	tc->set_allocated_pd_session_info(info);
}


void Serializer::set_ram_session(Capability_mapping *_cm,
								 Pb::Child_info *tc,
								 Child_info *_tc)
{
	DEBUG_THIS_CALL;
	Ram_session_info *_info = &_tc->ram_session->info;	
	Pb::Ram_session_info *info = new(_alloc) Pb::Ram_session_info();
	info->set_allocated_session_info(session_info(_cm, _info));

	Ram_dataspace *ram_dataspace = _info->ram_dataspaces;
	while(ram_dataspace) {
		add_ram_dataspace(_cm, info, &ram_dataspace->info);
		ram_dataspace = ram_dataspace->next();
	}

	tc->set_allocated_ram_session_info(info);		
}


void Serializer::set_cpu_session(Capability_mapping *_cm,
								 Pb::Child_info *tc,
								 Child_info *_tc)
{
	DEBUG_THIS_CALL;
	Cpu_session_info *_info = &_tc->cpu_session->info;	
	Pb::Cpu_session_info *info = new(_alloc) Pb::Cpu_session_info();
	info->set_allocated_session_info(session_info(_cm, _info));

	Cpu_thread *cpu_thread = _info->cpu_threads;
	while(cpu_thread) {
		add_cpu_thread(_cm, info, &cpu_thread->info);
		cpu_thread = cpu_thread->next();
	}

	tc->set_allocated_cpu_session_info(info);	
}


void Serializer::set_timer_session(Capability_mapping *_cm,
								   Pb::Child_info *tc,
								   Child_info *_tc)
{
	DEBUG_THIS_CALL;
	if(!_tc->timer_session) return;
	Timer_session_info *_info = &_tc->timer_session->info;
	
	Pb::Timer_session_info *info = new(_alloc) Pb::Timer_session_info();
	info->set_allocated_session_info(session_info(_cm, _info));
	info->set_sigh_badge(_info->sigh_badge);
	info->set_timeout(_info->timeout);
	info->set_periodic(_info->periodic);

	tc->set_allocated_timer_session_info(info);		
}


void Serializer::set_log_session(Capability_mapping *_cm,
								 Pb::Child_info *tc,
								 Child_info *_tc)
{
	DEBUG_THIS_CALL;
	if(!_tc->log_session) return;
	Log_session_info *_info = &_tc->log_session->info;
	
	Pb::Log_session_info *info = new(_alloc) Pb::Log_session_info();
	info->set_allocated_session_info(session_info(_cm, _info));

	tc->set_allocated_log_session_info(info);				
}


void Serializer::set_rm_session(Capability_mapping *_cm,
								Pb::Child_info *tc,
								Child_info *_tc)
{
	DEBUG_THIS_CALL;
	if(!_tc->rm_session) return;
	Rm_session_info *_info = &_tc->rm_session->info;

	Pb::Rm_session_info *info = new(_alloc) Pb::Rm_session_info();
	info->set_allocated_session_info(session_info(_cm, _info));

	Region_map *region_map = _info->region_maps;
	while(region_map) {
		add_region_map(_cm, info, &region_map->info);
		region_map = region_map->next();
	}

	tc->set_allocated_rm_session_info(info);			
}


void Serializer::set_rom_session(Capability_mapping *_cm,
								 Pb::Child_info *tc,
								 Child_info *_tc)
{
	DEBUG_THIS_CALL;
	if(!_tc->rom_session) return;
	Rom_session_info *_info = &_tc->rom_session->info;
	
	Pb::Rom_session_info *info = new(_alloc) Pb::Rom_session_info();
	info->set_allocated_session_info(session_info(_cm, _info));	
	info->set_dataspace_badge(_info->dataspace_badge);
	info->set_sigh_badge(_info->sigh_badge);

	tc->set_allocated_rom_session_info(info);		
}


void Serializer::add_region_map(Capability_mapping *_cm,
								Pb::Rm_session_info *rm_session_info,
								Region_map_info *_info)
{
	DEBUG_THIS_CALL;	
	Pb::Region_map_info *info = rm_session_info->add_region_map_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_size(_info->size);
	info->set_ds_badge(_info->ds_badge);
	info->set_sigh_badge(_info->sigh_badge);

	Attached_region *attached_region = _info->attached_regions;
	while(attached_region) {
		add_attached_region(_cm, info, &attached_region->info);
		attached_region = attached_region->next();
	}	
}


void Serializer::set_address_space(Capability_mapping *_cm,
								   Pb::Pd_session_info *pd_session_info,
								   Region_map_info *_info)
{
	DEBUG_THIS_CALL;
	Pb::Region_map_info *info = new(_alloc) Pb::Region_map_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_size(_info->size);
	info->set_ds_badge(_info->ds_badge);
	info->set_sigh_badge(_info->sigh_badge);

	Attached_region *attached_region = _info->attached_regions;
	while(attached_region) {
		add_attached_region(_cm, info, &attached_region->info);
		attached_region = attached_region->next();
	}
	pd_session_info->set_allocated_address_space(info);
}


void Serializer::set_stack_area(Capability_mapping *_cm,
								Pb::Pd_session_info *pd_session_info,
								Region_map_info *_info)
{
	DEBUG_THIS_CALL;
	Pb::Region_map_info *info = new(_alloc) Pb::Region_map_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_size(_info->size);
	info->set_ds_badge(_info->ds_badge);
	info->set_sigh_badge(_info->sigh_badge);

	Attached_region *attached_region = _info->attached_regions;
	while(attached_region) {
		add_attached_region(_cm, info, &attached_region->info);
		attached_region = attached_region->next();
	}
	pd_session_info->set_allocated_stack_area(info);
}


void Serializer::set_linker_area(Capability_mapping *_cm,
								 Pb::Pd_session_info *pd_session_info,
								 Region_map_info *_info)
{
	DEBUG_THIS_CALL;
	Pb::Region_map_info *info = new(_alloc) Pb::Region_map_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_size(_info->size);
	info->set_ds_badge(_info->ds_badge);
	info->set_sigh_badge(_info->sigh_badge);

	Attached_region *attached_region = _info->attached_regions;
	while(attached_region) {
		add_attached_region(_cm, info, &attached_region->info);
		attached_region = attached_region->next();
	}
	pd_session_info->set_allocated_linker_area(info);
}


void Serializer::add_attached_region(Capability_mapping *_cm,
									 Pb::Region_map_info *region_map_info,
									 Attached_region_info *_info)
{
	DEBUG_THIS_CALL;	
	Pb::Attached_region_info *info = region_map_info->add_attached_region_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));

	info->set_attached_ds_badge(_info->attached_ds_badge);
	info->set_size(_info->size);
	info->set_offset(_info->offset);
	info->set_rel_addr(_info->rel_addr);
	info->set_executable(_info->executable);
}


void Serializer::add_cpu_thread(Capability_mapping *_cm,
								Pb::Cpu_session_info *cpu_session_info,
								Cpu_thread_info *_info)
{
	DEBUG_THIS_CALL;
	Pb::Cpu_thread_info *info = cpu_session_info->add_cpu_thread_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_pd_session_badge(_info->pd_session_badge);
	info->set_name(_info->name.string());
	info->set_weight(std::to_string(_info->weight.value).c_str());
	info->set_utcb(_info->utcb);
	info->set_started(_info->started);
	info->set_paused(_info->paused);
	info->set_single_step(_info->single_step);
	info->set_affinity(_info->affinity.xpos()); // TODO add y
	info->set_sigh_badge(_info->sigh_badge);
//	info->set_ts(_info->ts) // TODO
}


void Serializer::add_ram_dataspace(Capability_mapping *_cm,
								   Pb::Ram_session_info *ram_session,
								   Ram_dataspace_info *_info)
{
	DEBUG_THIS_CALL;	
	Pb::Ram_dataspace_info *info = ram_session->add_ram_dataspace_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_size(_info->size);
	info->set_cached(_info->cached);
	info->set_timestamp(_info->timestamp);
	info->set_local_offset(0); // set an arbitrary which will be updated later.
}


void Serializer::add_signal_source(Capability_mapping *_cm,
								   Pb::Pd_session_info *pd_session,
								   Signal_source_info *_info)
{
	DEBUG_THIS_CALL;	
	Pb::Signal_source_info *info = pd_session->add_signal_source_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
}


void Serializer::add_signal_context(Capability_mapping *_cm,
									Pb::Pd_session_info *pd_session,
									Signal_context_info *_info)
{
	DEBUG_THIS_CALL;	
	Pb::Signal_context_info *info = pd_session->add_signal_context_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_signal_source_badge(_info->signal_source_badge);
	info->set_imprint(_info->imprint);
}


void Serializer::add_native_capability(Capability_mapping *_cm,
									   Pb::Pd_session_info *pd_session,
									   Native_capability_info *_info)
{
	DEBUG_THIS_CALL;	
	Pb::Native_capability_info *info = pd_session->add_native_capability_info();
	info->set_allocated_normal_info(normal_info(_cm, _info));
	info->set_ep_badge(_info->ep_badge);	
}


void Serializer::parse(Genode::Dataspace_capability compressed_ds_cap)
{
	DEBUG_THIS_CALL; PROFILE_THIS_CALL;

	Genode::uint32_t *compressed_ds_addr = _env.rm().attach(compressed_ds_cap);
	Genode::uint32_t compressed_size = *(compressed_ds_addr);
	Genode::uint32_t total_size = *(compressed_ds_addr+1);	
	Genode::log("compressed_size=", Genode::Hex(compressed_size),
				" uncompressed_size=", Genode::Hex(total_size));

	/* decompress */
	Genode::size_t uncompressed_ds_size = page_aligned_size(total_size);
	Genode::Ram_dataspace_capability uncompressed_ds_cap(
		_env.ram().alloc(uncompressed_ds_size));
	
	Genode::size_t compressed_offset = sizeof(Genode::uint32_t);	
	Genode::uint32_t *uncompressed_ds_addr = _env.rm().attach(uncompressed_ds_cap);
	uLongf uncompressed_size = total_size;

	int return_value = uncompress((Bytef *)uncompressed_ds_addr,
								  &uncompressed_size,
								  (Bytef *)(compressed_ds_addr+2),
								  compressed_size);

	_env.rm().detach(compressed_ds_addr);
	
	if(return_value != Z_OK) {
		Genode::error("Uncompression failed");
		throw Genode::Exception();
	}
	
	/* read protobuf */
	Genode::uint32_t pb_size = *((Genode::uint32_t*) uncompressed_ds_addr);	
	Genode::log("pb_size=",Genode::Hex(pb_size));

	/* parse protobuf in dataspace */
	Pb::Child_info *child_info = new(_alloc) Pb::Child_info();
	Genode::size_t pb_offset = sizeof(Genode::uint32_t);
	child_info->ParseFromArray(uncompressed_ds_addr + pb_offset, pb_size);
	
}

