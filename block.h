#ifndef __BLOCK__
#define __BLOCK__

#include <map>
#include <string>

#include "base_electronic_component.h"
#include "pin.h"

class block : public base_electronic_component
{
public:
	block(const std::string &name_ = "", const std::map<std::string, pin> &pins_ = std::map<std::string, pin>(), const double &width_ = 0.152, const double &length_ = 0, const unsigned &x = 0, const unsigned &y = 0);
	block(const block &ob);
	std::string get_name() const;
	const double get_x() const;
	const double get_y() const;
	void set_x(const unsigned &x_);
	void set_y(const unsigned &y_);
	const double get_width() const;
	const double get_length() const;
	static const double get_unit_width();
	static const double get_unit_length();
	static void set_unit_width(const double uw);
	static void set_unit_length(const double uh);
	const std::map<std::string, pin>& get_pins() const;
protected:
	static double unit_length;
	static double unit_width;
	std::map<std::string, pin> pins;
};

#endif