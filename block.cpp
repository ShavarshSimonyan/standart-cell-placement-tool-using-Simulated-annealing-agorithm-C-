#include "block.h"

double block::unit_length = 0.152;
double block::unit_width = 11 * 0.152;

block::block(const std::string &name_, const std::map<std::string, pin> &pins_, const double &width_, const double &length_, const unsigned &x_, const unsigned &y_) :base_electronic_component(name_, width_, length_, x_, y_),
																																									pins(pins_)
{}

block::block(const block &ob) : base_electronic_component(ob.get_name(), ob.get_width(), ob.get_length(), ob.get_x(), ob.get_y()),
pins(ob.get_pins())
{}

std::string block::get_name()const
{
	return name;
}

const double block::get_x() const
{
	return coordinates.get_x();
}

const double block::get_y() const
{
	return coordinates.get_y();
}

void block::set_x(const unsigned &x_)
{
	coordinates.set_x(x_);
}

void block::set_y(const unsigned &y_)
{
	coordinates.set_y(y_);
}

const double block::get_width() const
{
	return width;
}

const double block::get_length() const
{
	return length;
}

const double block::get_unit_width()
{
	return unit_width;
}

const double block::get_unit_length()
{
	return unit_length;
}

void block::set_unit_width(const double uw)
{
	unit_width = uw;
}

void block::set_unit_length(const double uh)
{
	unit_length = uh;
}

const std::map<std::string, pin>& block::get_pins() const
{
	return pins;
}