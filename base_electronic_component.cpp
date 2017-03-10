#include"base_electronic_component.h"

base_electronic_component::base_electronic_component(const std::string name_, const double &width_, const double &length_, const double &x_, const double &y_) :name(name_), coordinates(x_, y_),
width(width_), length(length_)
{}