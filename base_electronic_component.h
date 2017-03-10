#ifndef __ELECTRONIC_COMPONENT__
#define __ELECTRONIC_COMPONENT__

#include<string>

#include "point.h"

class base_electronic_component
{
public:
	base_electronic_component(const std::string name_ = "", const double &width_ = 0, const double &length_ = 0, const double &x = 0, const double &y = 0);
	virtual std::string get_name() const = 0;
	virtual const double get_x() const = 0;
	virtual const double get_y() const = 0;
	virtual void set_x(const unsigned &x_) = 0;
	virtual void set_y(const unsigned &y_) = 0;
	virtual const  double get_width() const = 0;
	virtual const double get_length() const = 0;
protected:
	std::string name;
	point<double> coordinates;
	double width;
	double length;
};

#endif