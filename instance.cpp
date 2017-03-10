#include "instance.h"

#include <algorithm>
#include <utility>

instance::instance(const std::string &ins_name_, const block & ob) :ins_name(ins_name_), ptr(&ob), pins(ob.get_pins())
{
	std::for_each(pins.begin(), pins.end(), [this](std::pair<const std::string, pin> &current_pin) {current_pin.second.set_instance_coordinates(this->get_coordinates());});
}

instance::instance(const instance & ob) : ins_name(ob.get_name()), ptr(ob.ptr), coordinates(ob.coordinates.get_x(), ob.coordinates.get_y()),pins(ob.pins)
{
	std::for_each(pins.begin(), pins.end(), [this](std::pair<const std::string, pin> &current_pin) {current_pin.second.set_instance_coordinates(this->get_coordinates());});
}

std::string instance::get_name()const
{
	return ins_name;
}

const unsigned instance::get_x() const
{
	return coordinates.get_x();
}
const unsigned instance::get_y() const
{
	return coordinates.get_y();
}

void instance::set_x(const unsigned &x)
{
	coordinates.set_x(x);
}

void instance::set_y(const unsigned &y)
{
	coordinates.set_y(y);
}

const std::map<std::string, pin>& instance::get_pins() const
{
	return pins;
}

const point<unsigned>& instance::get_coordinates()const
{
	return coordinates;
}

pin& instance::add_net(const std::string &pin_name, const unsigned &net_index)
{
	auto it = pins.find(pin_name);
	it->second.add_net(net_index);

	return it->second;
}