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
	struct General;
	struct Session_rpc;
	struct Normal_rpc;
	struct Normal_obj;
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


struct Rtcr::General
{
	bool const bootstrapped;

	General() : bootstrapped(false)
		{ }

	General(bool bootstrapped) : bootstrapped(bootstrapped)
		{ }

	void print(Genode::Output &output) const
		{
			Genode::print(output, "bootstrapped=", bootstrapped);
		}
};


/**
 * Struct to monitor session RPC objects
 */
struct Rtcr::Session_rpc : General
{
	Genode::String<160> creation_args;
	Genode::String<160> upgrade_args;

	Session_rpc() : General(), creation_args(""), upgrade_args("")
		{ }

	Session_rpc(const char* creation_args, const char* upgrade_args, bool bootstrapped)
		:
		General  (bootstrapped),
		creation_args (creation_args),
		upgrade_args  (upgrade_args)
		{ }

	void print(Genode::Output &output) const
		{
			Genode::print(output, "cargs='", creation_args, "', uargs='", upgrade_args, "', ");
			General::print(output);
		}
};


/**
 * Struct to monitor normal RPC object (e.g. Region map, CPU thread)
 */
struct Rtcr::Normal_rpc : General
{
	// Insert common members

	Normal_rpc() : General()
		{ }

	Normal_rpc(bool bootstrapped)
		:
		General(bootstrapped)
		{ }

};


/**
 * Struct to store information about RPC function invokation (e.g. parameter, return value)
 */
struct Rtcr::Normal_obj : General
{
	// Insert common members

	Normal_obj() : General()
		{ }

	Normal_obj(bool bootstrapped)
		:
		General(bootstrapped)
		{ }

};


#endif /* _RTCR_INFO_STRUCTS_H_ */
