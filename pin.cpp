#include"pin.h"

pin::pin(const std::string name_, const double x_, const double y_) : base_electronic_component(name_, 0, 0, x_, y_)
{}

pin::pin(const pin &ob) : base_electronic_component(ob.name, ob.width, ob.length, ob.coordinates.get_x(), ob.coordinates.get_y()), p_instaceCoordinates(ob.p_instaceCoordinates)
{}

std::string pin::get_name()const
{
	return name;
}

const double pin::get_x() const
{
	return coordinates.get_x() + p_instaceCoordinates->get_x();
}

const double pin::get_y() const
{
	return coordinates.get_y() + p_instaceCoordinates->get_y();
}

void pin::set_x(const unsigned &x_)
{
	coordinates.set_x(x_);
}

void pin::set_y(const unsigned &y_)
{
	coordinates.set_y(y_);
}

const double pin::get_width() const
{
	return width;
}

const double pin::get_length() const
{
	return length;
}

const std::vector<unsigned>& pin::get_nets() const
{
	return nets;
}

void pin::add_net(const unsigned &net_)
{
	nets.push_back(net_);
}

void pin::set_instance_coordinates(const point<unsigned> &ob)
{
	p_instaceCoordinates = &ob;
}