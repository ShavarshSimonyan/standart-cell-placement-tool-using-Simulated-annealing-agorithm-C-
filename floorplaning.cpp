#include "floorplaning.h"

#include <ctime>
#include <cmath>
#include <iostream>
#include <queue>
#include <functional> 

const double floorplaning::INIT_TEMP = 4000000;
const double floorplaning::FINAL_TEMP = 0.1;

floorplaning::floorplaning(integrated_circuit &ob, const std::string &outputName_, bool whole_circuit_bbox_, const bool do_initial_palcement_) :pIntegratedCircuit(&ob),
													outputName(outputName_),
													layer_width(0),
													layer_length(0),
													initial_coast(0),
													bad_attempts(0),
													layerSizeCoefficient(5),
													max_number_of_bad_attempts(7),
													number_of_instance_attempt(100),
													unit_length(block::get_unit_length()),
													unit_width(block::get_unit_width()),
													whole_circuit_bbox(whole_circuit_bbox_),
													do_initial_palcement(do_initial_palcement_)
{
	if (pIntegratedCircuit->instances.size() > 300)
	{
		double k1 = (pIntegratedCircuit->instances.size() / 200);
		double k2 = (pIntegratedCircuit->instances.size() / 3000);
		number_of_instance_attempt = ((k1 * 100) + (k2 * 700)) / 4;

		if (pIntegratedCircuit->instances.size() >= 3000)
			number_of_instance_attempt = 700;

		if (whole_circuit_bbox)
			k1 *= 0.7;

		max_number_of_bad_attempts += k1;
	}

	layer_length_width_values();

	bboxTopLeft.set_x(0);
	bboxTopLeft.set_y(0);

	bboxBottomRight.set_x(layer_length);
	bboxBottomRight.set_y(layer_width);

	bboxX1 = &bboxBottomRight;
	bboxY1 = &bboxBottomRight;

	bboxX2 = &bboxTopLeft;
	bboxY2 = &bboxTopLeft;
}

void floorplaning::layer_length_width_values()
{
	double l = 0, w = 0;
	for (unsigned i = 0; i < pIntegratedCircuit->instances.size(); ++i)
	{
		l += pIntegratedCircuit->instances[i].ptr->get_length();
		w += pIntegratedCircuit->instances[i].ptr->get_width();
	}
	std::cout << l << ' ' << w<<std::endl;//temp
	l *= layerSizeCoefficient / unit_length;
	w *= layerSizeCoefficient / unit_width;

	layer_length = l;
	layer_width = w;

}

double floorplaning::count_cost_of_circuit()
{
	double result = 0;
	for (unsigned i = 0; i < pIntegratedCircuit->netlist.size(); ++i)
	{
		pIntegratedCircuit->netlistCurrentValues[i] = count_net_value(i);
		result += pIntegratedCircuit->netlistCurrentValues[i];
	}

	return result;
}

double floorplaning::new_cost(instance &ob, const unsigned &x, const unsigned &y, std::vector<std::pair<unsigned, double> > &changed_nets)
{
	unsigned current_x = ob.get_x(), current_y = ob.get_y();
	ob.set_x(x);
	ob.set_y(y);

	changed_nets.reserve(pIntegratedCircuit->netlistCurrentValues.size());

	static std::vector<bool> counted_nets(pIntegratedCircuit->netlistCurrentValues.size());
	counted_nets.assign(counted_nets.size(), false);
	double result = 0;
	for (auto it = ob.get_pins().begin(); it != ob.get_pins().end(); ++it)//picking each pin of instacne
	{
		const std::vector<unsigned> &arr = it->second.get_nets();//taking the nets in which current pin is participating
		for (unsigned i = 0; i < arr.size(); ++i)//for each of the net counting it's value if we haven't counted it yet
		{
			if (!counted_nets[arr[i]])
			{
				counted_nets[arr[i]] = true;
				changed_nets.push_back(std::make_pair(arr[i], count_net_value(arr[i])));
				result += changed_nets.back().second;
			}
		}
	}

	ob.set_x(current_x);
	ob.set_y(current_y);

	return result;
}

double floorplaning::current_cost(const instance &ob)
{
	static std::vector<bool> counted_nets(pIntegratedCircuit->netlistCurrentValues.size());
	counted_nets.assign(counted_nets.size(), false);
	double result = 0;

	for (auto it = ob.get_pins().begin(); it != ob.get_pins().end(); ++it)//picking each pin of instacne
	{
		const std::vector<unsigned> &arr = it->second.get_nets();//taking the nets in which current pin is participating
		for (unsigned i = 0; i < arr.size(); ++i)// for each of the net counting it's value if we haven't counted it yet
		{
			if (!counted_nets[arr[i]])
			{
				counted_nets[arr[i]] = true;
				result += pIntegratedCircuit->netlistCurrentValues[arr[i]];
			}
		}
	}

	return result;
}

double floorplaning::count_net_value(const unsigned &net_index)
{
	double result = 0;
	if (pIntegratedCircuit->netlist[net_index].size() < 2)
		return result;

	std::priority_queue <int, std::vector<double>, std::greater<double> > q;

	for (unsigned j = 0; j < pIntegratedCircuit->netlist[net_index].size() - 1; ++j)
		for (unsigned i = j + 1; i < pIntegratedCircuit->netlist[net_index].size(); ++i)
			q.push(manhatten_distance(point<double>(pIntegratedCircuit->netlist[net_index][j]->get_x(), pIntegratedCircuit->netlist[net_index][j]->get_y()), 
				point<double>(pIntegratedCircuit->netlist[net_index][i]->get_x(), pIntegratedCircuit->netlist[net_index][i]->get_y())));

	for (unsigned k = 0; k < pIntegratedCircuit->netlist[net_index].size() - 1; ++k)
	{
		result += q.top();
		q.pop();
	}
	return result * pIntegratedCircuit->netCoefficient[net_index];
}

double floorplaning::simulated_annealing()
{
	if (do_initial_palcement)
		initial_placement();

	else if (!do_initial_palcement && !whole_circuit_bbox)
		max_number_of_bad_attempts *= double(1 / 0.7);

	double temp = INIT_TEMP, cost_diff = 0;

	while (temp > FINAL_TEMP)
	{
		while (inner_loop_criterion())
		{
			perturb(false, cost_diff);

			if (cost_diff < 0)
				perturb(true, cost_diff);
			else if (accept(cost_diff, temp))
				perturb(true, cost_diff);
			
		}
		update_temperature(temp);
	}

	return count_results();
}

void floorplaning::initial_placement()
{
	pIntegratedCircuit->layer.resize(layer_width);
	for (unsigned i = 0; i < layer_width; ++i)
	{
		pIntegratedCircuit->layer[i].resize(layer_length, nullptr);
	}

	unsigned x = 0, y = 0;
	for (unsigned i = 0; i < pIntegratedCircuit->instances.size(); ++i)
		if (random_placement(pIntegratedCircuit->instances[i], x, y))
			move_instance(pIntegratedCircuit->instances[i], x, y);

	pIntegratedCircuit->creat_output();
	pIntegratedCircuit->creat_def(outputName);
	initial_coast = count_cost_of_circuit();
	update_bbox();
	temperatureCoefficient = initial_coast / INIT_TEMP;
}

void floorplaning::get_space(const unsigned &l, const unsigned &w, std::vector<point<unsigned> > &free_space)
{
	for (unsigned y = bboxTopLeft.get_y(); y < bboxBottomRight.get_y(); ++y)
		for (unsigned x = bboxTopLeft.get_x(); x < bboxBottomRight.get_x(); ++x)
			if (if_space_is_free(x, y, l, w))
			{
				free_space.push_back(point<unsigned>(x, y));
				return;
			}
}

bool floorplaning::random_placement(instance & ob, unsigned &x, unsigned &y)
{
	unsigned len = 0, wid = 0, number_of_random_attempts = 0;
	const unsigned max_number_of_random_attempts = (bboxBottomRight.get_x() - bboxTopLeft.get_x()) * (bboxBottomRight.get_y() - bboxTopLeft.get_y());

	len = ob.ptr->get_length() / unit_length;
	wid = ob.ptr->get_width() / unit_width;

	unsigned x_diff = bboxBottomRight.get_x() - bboxTopLeft.get_x() - len;
	unsigned y_diff = bboxBottomRight.get_y() - bboxTopLeft.get_y() - wid;
	while (!if_space_is_free(x, y, len, wid))
	{
		x = rand() % x_diff + bboxTopLeft.get_x();
		y = rand() % y_diff + bboxTopLeft.get_y();
		if (++number_of_random_attempts == max_number_of_random_attempts)
			break;
	}
	
	if (number_of_random_attempts == max_number_of_random_attempts)
	{
		std::vector<point<unsigned> > free_space;
		get_space(len, wid, free_space);
		if (free_space.empty())
		{
			return false;
		}

		x = free_space[0].get_x();
		y = free_space[0].get_y();
	}

	return true;
}

bool floorplaning::if_space_is_free(const unsigned &x, const unsigned &y, const unsigned &l, const unsigned &w)
{
	for (unsigned i = y; i < y + w; ++i)
		for (unsigned k = x; k < x + l; ++k)
			if (pIntegratedCircuit->layer[i][k] != nullptr)
				return false;

	return true;
}

void floorplaning::move_instance(instance & ob, const unsigned &x, const unsigned &y)
{
	unsigned len = ob.ptr->get_length() / unit_length;
	unsigned wid = ob.ptr->get_width() / unit_width;
	clear_space(x, y, len, wid);
	place_instance(ob, x, y, len, wid);
}

void floorplaning::place_instance(instance & ob, const unsigned &x, const unsigned &y, const unsigned &l, const unsigned &w)
{
	ob.set_x(x);
	ob.set_y(y);

	for (unsigned i = y; i < y + w; ++i)
		for (unsigned k = x; k < x + l; ++k)
			pIntegratedCircuit->layer[i][k] = &ob;
}

void floorplaning::clear_space(const unsigned &x, const unsigned &y, const unsigned &l, const unsigned &w)
{
	for (unsigned i = y; i < y + w; ++i)
		for (unsigned k = x; k < x + l; ++k)
			pIntegratedCircuit->layer[i][k] = nullptr;

}

double floorplaning::manhatten_distance(const point<double> & x, const point<double> & y)
{
	double x_diff = x.get_x() - y.get_x(), y_diff = x.get_y() - y.get_y();
	return (abs(x_diff /* unit_width*/) + abs(y_diff /* unit_length*/));
}

void floorplaning::update_temperature(double &temp)
{
	if (temp > (INIT_TEMP / 4) && temp < (INIT_TEMP / 2))
	{
		temp *= 0.95;
		return;
	}

	temp *= 0.80;
}

bool floorplaning::inner_loop_criterion()
{
	static int number_of_attempts = pIntegratedCircuit->instances.size() * number_of_instance_attempt;

	if (number_of_attempts <= 0)
	{
		bad_attempts = 0;
		number_of_attempts = pIntegratedCircuit->instances.size() * number_of_instance_attempt;
		return false;
	}
	
	if (bad_attempts >= max_number_of_bad_attempts)
	{
		bad_attempts = 0;
		return false;
	}

	--number_of_attempts;

	return true;
}

void floorplaning::perturb(const bool b, double &coast_diff)
{
	static unsigned i = 0, x = 0, y = 0;
	static std::vector<std::pair<unsigned, double> >changed_nets;
	static bool palcement_was_sucessfull = false;
	static const unsigned number_of_instances = pIntegratedCircuit->instances.size();

	if (!b)
	{
		if ((number_of_instances <= i))
			i = 0;
		
		palcement_was_sucessfull = false;
		changed_nets.clear();
		coast_diff = 0;
		if (!whole_circuit_bbox)
			current_bbox(i);
		if (random_placement(pIntegratedCircuit->instances[i], x, y))
		{
			coast_diff = current_cost(pIntegratedCircuit->instances[i]) - new_cost(pIntegratedCircuit->instances[i], x, y, changed_nets);
			palcement_was_sucessfull = true;
			++i;

			if (coast_diff >= 0)
				++bad_attempts;
			else
				bad_attempts = 0;

			return;
		}
		++bad_attempts;
	}
	else if (b && palcement_was_sucessfull)
	{
		move_instance(pIntegratedCircuit->instances[i - 1], x, y);
		if (whole_circuit_bbox && !(rand() % 13))
			update_bbox();
		update_netlist_values(changed_nets);
	}
}

void floorplaning::update_netlist_values(const std::vector<std::pair<unsigned, double> > &changed_nets)
{
	for (unsigned i = 0; i < changed_nets.size(); ++i) 
		pIntegratedCircuit->netlistCurrentValues[changed_nets[i].first] = changed_nets[i].second;
}

bool floorplaning::accept(const double &coast_diff, const double & temp)
{
	double r = static_cast <double> (rand()) / static_cast <double> (RAND_MAX);
	if (r > exp(-coast_diff / temp * temperatureCoefficient) )
		return true;

	return false;
}

void floorplaning::update_bbox()
{
	if (bboxX1->get_x() == bboxTopLeft.get_x() && bboxY1->get_y() == bboxTopLeft.get_y()
		&& bboxX2->get_x() == bboxBottomRight.get_x() && bboxY2->get_y() == bboxBottomRight.get_y())
		return;

	bboxTopLeft.set_x(pIntegratedCircuit->instances[0].get_x());
	bboxTopLeft.set_y(pIntegratedCircuit->instances[0].get_y());

	bboxBottomRight.set_x(pIntegratedCircuit->instances[0].get_x());
	bboxBottomRight.set_y(pIntegratedCircuit->instances[0].get_y());

	for (unsigned i = 1; i < pIntegratedCircuit->instances.size(); ++i)
	{
		if (pIntegratedCircuit->instances[i].get_x() < bboxTopLeft.get_x())
		{
			bboxTopLeft.set_x(pIntegratedCircuit->instances[i].get_x());
			bboxX1 = &(pIntegratedCircuit->instances[i].get_coordinates());
		}
		else if (pIntegratedCircuit->instances[i].get_x() > bboxBottomRight.get_x())
		{
			bboxBottomRight.set_x(pIntegratedCircuit->instances[i].get_x());
			bboxX2 = &(pIntegratedCircuit->instances[i].get_coordinates());
		}

		if (pIntegratedCircuit->instances[i].get_y() < bboxTopLeft.get_y())
		{
			bboxTopLeft.set_y(pIntegratedCircuit->instances[i].get_y());
			bboxY1 = &(pIntegratedCircuit->instances[i].get_coordinates());
		}
		else if (pIntegratedCircuit->instances[i].get_y() > bboxBottomRight.get_y())
		{
			bboxBottomRight.set_y(pIntegratedCircuit->instances[i].get_y());
			bboxY2 = &(pIntegratedCircuit->instances[i].get_coordinates());
		}
	}
		
}

void floorplaning::current_bbox(const unsigned &index)
{
	static std::vector<bool> counted_nets(pIntegratedCircuit->netlistCurrentValues.size());
	counted_nets.assign(counted_nets.size(), false);
	instance &ins = pIntegratedCircuit->instances[index];

	bboxTopLeft.set_x(pIntegratedCircuit->instances[index].get_x());
	bboxTopLeft.set_y(pIntegratedCircuit->instances[index].get_y());

	bboxBottomRight.set_x(pIntegratedCircuit->instances[index].get_x());
	bboxBottomRight.set_y(pIntegratedCircuit->instances[index].get_y());
	
	for (auto it = ins.get_pins().begin(); it != ins.get_pins().end(); ++it)//picking each pin of instacne
	{
		const std::vector<unsigned> &arr = it->second.get_nets();//taking the nets in which current pin is participating
		for (unsigned i = 0; i < arr.size(); ++i)// for each of the net counting it's value if we haven't counted it yet
		{
			if (!counted_nets[arr[i]])
			{
				counted_nets[arr[i]] = true;

				for (unsigned j = 0; j < pIntegratedCircuit->instanceConnection[i].size(); ++j)
				{
					if (pIntegratedCircuit->instanceConnection[i][j]->get_x() < bboxTopLeft.get_x())
						bboxTopLeft.set_x(pIntegratedCircuit->instanceConnection[i][j]->get_x());
					else if (pIntegratedCircuit->instanceConnection[i][j]->get_x() > bboxBottomRight.get_x())
						bboxBottomRight.set_x(pIntegratedCircuit->instanceConnection[i][j]->get_x());
					if (pIntegratedCircuit->instanceConnection[i][j]->get_y() < bboxTopLeft.get_y())
						bboxTopLeft.set_y(pIntegratedCircuit->instanceConnection[i][j]->get_y());
					else if (pIntegratedCircuit->instanceConnection[i][j]->get_y() > bboxBottomRight.get_y())
						bboxBottomRight.set_y(pIntegratedCircuit->instanceConnection[i][j]->get_y());
				}
			}
		}
	}
}

double floorplaning::count_results()
{
	bboxTopLeft.set_x(0);
	bboxTopLeft.set_y(0);
	bboxBottomRight.set_x(0);
	bboxBottomRight.set_y(0);
	update_bbox();

	double layer_current_area = (bboxBottomRight.get_x() - bboxTopLeft.get_x())*(bboxBottomRight.get_y() - bboxTopLeft.get_y());
	double layer_initial_area = layer_length * layer_width;
	double cost = count_cost_of_circuit();
	double improvement = (1 - cost / initial_coast) * 100;
	
	std::cout << "the improvement is " << improvement << "%\n" << "initial coast " << initial_coast
		<< "\ncurrent_coast is " << cost << "\nthe area reduced "
		<< (1 - layer_current_area / layer_initial_area) * 100 << "%\n";

	pIntegratedCircuit->creat_output();

	return improvement;
}