#ifndef __INSTANCE__
#define __INSTANCE__

#include <string>

#include"block.h"
#include"point.h"
#include"pin.h"

class instance
{
public:
	instance(const std::string &ins_name_, const block &);
	instance(const instance &);
	const unsigned get_x() const;
	const unsigned get_y() const;
	void set_x(const unsigned &);
	void set_y(const unsigned &);
	const std::map<std::string, pin>& get_pins() const;
	std::string get_name()const;
	const point<unsigned>& get_coordinates() const;
	pin& add_net(const std::string &, const unsigned &);

	block const * const ptr;
private:
	point<unsigned> coordinates;
	std::string ins_name;
	std::map<std::string, pin> pins;
};

#endif