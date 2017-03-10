#ifndef __FLOORPLANING__
#define __FLOORPLANING__

#include <utility>

#include "Integrated_Circuit.h"

class floorplaning
{
public:
	floorplaning(integrated_circuit &ob, const std::string &outputName_, bool whole_circuit_bbox_ = true, const bool do_initial_palcement_ = true);
	double simulated_annealing();
private:
	void layer_length_width_values();
	double count_cost_of_circuit();
	double new_cost(instance &, const unsigned &, const unsigned &, std::vector<std::pair<unsigned, double> > &);
	double current_cost(const instance &);
	double count_net_value(const unsigned &);
	void initial_placement();
	bool random_placement(instance &, unsigned &, unsigned &);
	void get_space(const unsigned &, const unsigned &, std::vector<point<unsigned> > &);
	void move_instance(instance &, const unsigned &, const unsigned &);
	void place_instance(instance &, const unsigned &, const unsigned &, const unsigned &, const unsigned &);
	bool if_space_is_free(const unsigned &, const unsigned &, const unsigned &, const unsigned &);
	void clear_space(const unsigned &, const unsigned &, const unsigned &, const unsigned &);
	double  manhatten_distance(const point<double> &, const point<double> &);
	bool inner_loop_criterion();
	void update_temperature(double &);
	void perturb(const bool, double &);
	void update_netlist_values(const std::vector<std::pair<unsigned, double> > &);
	bool accept(const double &, const double &);
	void update_bbox();
	double count_results();

	void current_bbox(const unsigned &);

	integrated_circuit * const pIntegratedCircuit;
	
	double initial_coast;
	unsigned layer_width;
	unsigned layer_length;

	unsigned bad_attempts;
	unsigned max_number_of_bad_attempts;
	double number_of_instance_attempt;

	static const double  INIT_TEMP;
	static const double FINAL_TEMP;

	point<unsigned> bboxTopLeft;
	point<unsigned> bboxBottomRight;
	point<unsigned> const *bboxX1;
	point<unsigned> const *bboxX2;
	point<unsigned> const *bboxY1;
	point<unsigned> const *bboxY2;

	double unit_length;
	double unit_width;

	double temperatureCoefficient;
	double layerSizeCoefficient;

	std::string outputName;

public:

	bool whole_circuit_bbox;
	bool do_initial_palcement;
};

#endif