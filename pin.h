#ifndef __PIN__
#define __PIN__

#include<string>
#include<vector>

#include"base_electronic_component.h"

class pin : public base_electronic_component
{
public:
	pin(const std::string name_ = "", const double x = 0, const double y = 0);
	pin(const pin &);
	std::string get_name() const;
	const double get_x() const;
	const double get_y() const;
	void set_x(const unsigned &);
	void set_y(const unsigned &);
	const double get_width() const;
	const double get_length() const;
	const std::vector<unsigned>& get_nets() const;
	void add_net(const unsigned &);
	void set_instance_coordinates(const point<unsigned> &);
private:
	std::vector<unsigned> nets;
	point<unsigned> const* p_instaceCoordinates;
};

#endif