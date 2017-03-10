#ifndef __INTEGRATED_CIRCUIT__
#define __INTEGRATED_CIRCUIT__

#include<string>
#include<map>
#include<fstream>
#include<vector>

#include"point.h"
#include"base_electronic_component.h"
#include"block.h"
#include"instance.h"

class integrated_circuit
{
public:
	friend class floorplaning;
	integrated_circuit(std::string input_lef_file = "input.lef", std::string input_verilog_file = "input.v");
	~integrated_circuit();

	void creat_def(const std::string &) const;
private:
	void creat_output();
	bool lef_parser(const std::string &);
	bool verilog_parser(const std::string &);
	void halt(const std::string &) const;
	std::map<std::string, block> blocks;
	std::vector<instance> instances;
	std::vector<std::vector<instance *> > layer;

	std::vector<double> netlistCurrentValues;
	std::vector<unsigned> netCoefficient;
	std::vector<std::vector<const pin*> > netlist;
	std::vector<std::vector<const instance*> > instanceConnection;

	std::vector<std::pair<std::string, double> > specialNetsCoefficient;

	std::string output;
};

#endif