#include "Integrated_Circuit.h"

#include <utility>
#include <algorithm>
#include <streambuf>
#include <fstream>
#include <cstdio>
#include <locale>
#include <algorithm>


const static unsigned units_distance_microns = 1000;

integrated_circuit::integrated_circuit(std::string input_lef_file, std::string input_verilog_file/*, const double unit_width_, const double unit_length_*/) //:unit_width(unit_width_), unit_length(unit_length_)
{
	specialNetsCoefficient.push_back(std::make_pair("CLK",0.09));

	if (!lef_parser(input_lef_file))
		halt(input_lef_file);

	if (!verilog_parser(input_verilog_file))
		halt(input_verilog_file);

	netlistCurrentValues.resize(netlist.size(), 0);
}

integrated_circuit::~integrated_circuit()
{}

bool integrated_circuit::verilog_parser(const std::string &verilog_file)
{
	std::ifstream fin(verilog_file);
	if (!fin.good())
		return false;

	std::string input((std::istreambuf_iterator<char>(fin)),
		std::istreambuf_iterator<char>());

	unsigned num_of_instances = 0;
	for (auto it = blocks.cbegin(); it != blocks.cend(); ++it)
		for (size_t pos = input.find(it->first, 0); pos != std::string::npos; pos = input.find(it->first, input.find(';', pos + it->first.size())))
			++num_of_instances;

	instances.reserve(num_of_instances);//this reservation is needed to escape this vector realocation, 
										//otherwise there will be null pointers in the netlist that will bring to crash!!!!
	unsigned i = 0, n = 0, j = 0;
	std::map<std::string, unsigned> net_to_int;
	std::string current_net_name, current_pin_name;
	const unsigned final_index = input.find("endmodule");
	for (auto it = blocks.cbegin(); it != blocks.cend(); ++it)
	{
		for (size_t pos = input.find(it->first, 0); pos < final_index && pos != std::string::npos; pos = input.find(it->first, n))
		{
			n = input.find(';', pos + it->first.size());
			bool a = (input[pos - 1] == ' ' || input[pos - 1] == '\n'), b = (input[pos + it->first.size()] == ' ' || input[pos + it->first.size()] == '\n');
			if (!a || !b)// if current block's name is part of the other block name then skip that case
				continue;

			j = input.find_first_not_of(' ', pos + it->first.size() + 1);// from j + 1 to
			i = input.find_first_of(" (", j);		// i - 1 is the instance name		

			instances.push_back(instance(input.substr(j, i - j), it->second));

			for (i = input.find_first_not_of(" .(\n", i)/*pin name's first index*/; i < n; i = input.find_first_not_of(" .,)\n", j))// in this loop netlist is being created
			{
				j = input.find_first_of(" (", i + 1);/*pin name's last + 1 index*/
				current_pin_name = input.substr(i, j - i);

				i = input.find_first_not_of("( \n", j);//net name's first index
				j = j = input.find_first_of(" )", i);/*net name's last + 1 index*/
				current_net_name = input.substr(i, j - i);
				std::transform(current_net_name.begin(), current_net_name.end(), current_net_name.begin(), toupper);
				if (current_net_name == "VSS" || current_net_name == "VDD")
					continue;

				auto net_to_int_it = net_to_int.find(current_net_name);
				if (net_to_int_it == net_to_int.end())
				{
					net_to_int[current_net_name] = net_to_int.size();

					netlist.resize(netlist.size() + 1);
					netlist.back().push_back(&(instances.back().add_net(current_pin_name, netlist.size() - 1)));

					instanceConnection.resize(instanceConnection.size() + 1);
					instanceConnection.back().push_back(&(instances.back()));
				}
				else
				{
					netlist[net_to_int_it->second].push_back(&(instances.back().add_net(current_pin_name, net_to_int_it->second)));
					instanceConnection[net_to_int_it->second].push_back(&(instances.back()));
				}

			}
			
		} 
		
	}
	netCoefficient.resize(netlist.size(), 1);

	for (unsigned i = 0; i < specialNetsCoefficient.size(); ++i)
		netCoefficient[net_to_int[specialNetsCoefficient[i].first]] = specialNetsCoefficient[i].second;

	return true;
}

bool integrated_circuit::lef_parser(const std::string &lef_file)
{
	std::ifstream fin(lef_file);
	if (!fin.good())
		return false;

	std::string input((std::istreambuf_iterator<char>(fin)),
		std::istreambuf_iterator<char>());

	unsigned n = 0, i = 0;
	std::string instance_identifier = "MACRO", block_name, pin_name, x1 , y1, x2, y2, strLength, strWidth;
	for (size_t pos = input.find(instance_identifier, 0); pos != std::string::npos; pos = input.find(instance_identifier, n))
	{
		n = input.find_first_not_of(' ', pos + instance_identifier.size() + 1);// first index of block name
		i = input.find_first_of(" \n", n);// last index of block name
		block_name = input.substr(n, i - n);

		n = input.find("SIZE", i);
		i = input.find_first_not_of(' ', n + 4);
		n = input.find_first_of(' ', i);
		strLength = input.substr(i, n - i);

		i = input.find_first_not_of(" BY", n + 1);
		n = input.find_first_of(" ;", i);
		strWidth = input.substr(i, n - i);
		std::map<std::string, pin> pins;
		n = input.find(block_name, i);
		for (unsigned j = input.find("PIN", i); j < n; j = input.find("PIN", j))
		{
			i = input.find_first_not_of(' ', j + 3);
			j = input.find_first_of(" \n", i);
			pin_name = input.substr(i, j - i);
			
			i = input.find("LAYER M1", j);
			j = input.find("RECT", i);

			i = input.find_first_not_of(' ', j + 4);
			j = input.find_first_of(' ', i);
			x1 = input.substr(i, j - i);

			i = input.find_first_not_of(' ', j + 1);
			j = input.find_first_of(' ', i);
			y1 = input.substr(i, j - i);

			i = input.find_first_not_of(' ', j + 1);
			j = input.find_first_of(' ', i);
			x2 = input.substr(i, j - i);

			i = input.find_first_not_of(' ', j + 1);
			j = input.find_first_of(" ;", i);
			y2 = input.substr(i, j - i);

			pin p(pin_name,(std::stod(x1, 0) + std::stod(x2, 0)) / 2, (std::stod(y1, 0) + std::stod(y2, 0)) / 2);
			pins[pin_name] = p;
		}
		
		block bl(block_name, pins, std::stod(strWidth), std::stod(strLength));
		blocks[block_name] = bl;
	}

	return true;
}

void integrated_circuit::creat_output()
{
	double len = layer[0].size() * units_distance_microns * block::get_unit_length(), wid = layer.size() * units_distance_microns * block::get_unit_width();
	std::string str = "#\nVERSION 5.6 ;\nDIVIDERCHAR \"/\" ;\nBUSBITCHARS \"[]\" ;\nDESIGN fib ;\nTECHNOLOGY saed32 ;\nUNITS DISTANCE MICRONS 1000 ;\n";
	str += "DIEAREA ( 0 0 ) ( " + std::to_string(wid)  + ' ' + std::to_string(len) + " ) ;\n";
	str += "COMPONENTS " + std::to_string(instances.size()) + " ;\n";
	for (auto i = instances.begin(); i != instances.end(); ++i)
	{
		str += " - " + (*i).get_name() + ' ' + (*i).ptr->get_name() + " + PLACED ( "
			 + std::to_string((*i).get_x() * units_distance_microns  * block::get_unit_length()) + ' '
			 + std::to_string((*i).get_y() * units_distance_microns * block::get_unit_width()) + " ) FN ;\n";
	}
	str += "END COMPONENTS\nEND DESIGN";

	output = str;
}

void integrated_circuit::creat_def(const std::string &outputName) const
{
	std::ofstream fout(outputName);
	fout << output;
}

void integrated_circuit::halt(const std::string &file_name) const
{
	std::string error_message = "Can't open " + file_name + " file";
	perror(error_message.c_str());
	exit(1);
}





