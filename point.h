#ifndef __POINT__
#define __POINT__

template<typename T>
class point
{
public:
	point(const T &x_ = 0, const T &y_ = 0);
	point(const point &ob);
	void set_x(const T &x_);
	void set_y(const T &y_);
	const T get_x() const;
	const T get_y() const;
private:
	T x;
	T y;
};

template<typename T>
point<T>::point(const T &x_, const T &y_) :x(x_), y(y_)
{}

template<typename T>
point<T>::point(const point & ob) : x(ob.get_x()), y(ob.get_y())
{}

template<typename T>
const T point<T>::get_x()const
{
	return x;
}

template<typename T>
const T point<T>::get_y()const
{
	return y;
}

template<typename T>
void point<T>::set_x(const T &x_)
{
	x = x_;
}

template<typename T>
void point<T>::set_y(const T &y_)
{
	y = y_;
}

#endif