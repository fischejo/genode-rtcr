/*
 * \brief  Structs for monitor's objects
 * \author Denis Huber
 * \date   2016-11-21
 */

#ifndef _RTCR_INFO_STRUCTS_H_
#define _RTCR_INFO_STRUCTS_H_

/* Genode includes */
#include <util/list.h>
#include <util/string.h>

namespace Rtcr {
	template<typename T> class Simple_counter;

	struct Normal_info;
	struct Session_info;
}

template<typename T>
class Rtcr::Simple_counter
{
private:
	Genode::size_t const current_id;
	static Genode::size_t get_id()
		{
			static Genode::size_t count = 0;
			return count++;
		}

public:
	Simple_counter() : current_id(get_id()) { }
	// Do not use copy ctor, because Genode::print(Output&, HEAD const&, TAIL ...) uses pass-by-value
	// which copies the object inherited from Simple_counter for the function call
	//Simple_counter(const Simple_counter&) : current_id(get_id()) { }
	//~Simple_counter() { }

	Genode::size_t id() const { return current_id; }
};

struct Rtcr::Normal_info
{
	bool i_bootstrapped;
	Genode::uint16_t i_badge;
	Genode::uint16_t i_kcap;	 /* only filled by serializer */

	Normal_info(Genode::uint16_t badge) : i_badge(badge) {}

	Normal_info() {}
	
	void print(Genode::Output &output) const {
		Genode::print(output,
					  "bootstrapped=", i_bootstrapped,
					  " badge=", i_badge);
	}
	
};


struct Rtcr::Session_info : Rtcr::Normal_info
{
	Genode::String<160> i_creation_args;
	Genode::String<160> i_upgrade_args;

	Session_info(const char* creation_args, Genode::uint16_t badge)
		:
		Normal_info(badge),
		i_creation_args (creation_args) { }

	Session_info() {};
	
	void print(Genode::Output &output) const {
		Normal_info::print(output);		
		Genode::print(output,
					  ", cargs='", i_creation_args,
					  "', uargs='", i_upgrade_args, "'");

	}
};




#endif /* _RTCR_INFO_STRUCTS_H_ */
