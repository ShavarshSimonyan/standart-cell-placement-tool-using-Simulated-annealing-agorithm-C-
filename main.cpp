#include<vector>
#include<iostream>
#include<ctime>

#include"floorplaning.h"

int main()
{
	std::clock_t tStart = std::clock();

	std::string lefFile, verilogFile, outputDefFile;
	std::cout << "Enter input (LEF) file name ";
	std::cin >> lefFile;

	std::cout << "Enter input (Verilog) file name ";
	std::cin >> verilogFile;

	std::cout << "Enter  output file name ";
	std::cin >> outputDefFile;
	
	double uw = 0, uh = 0;

	std::cout << "unit height ";
	std::cin >> uh;

	std::cout << "unit width ";
	std::cin >> uw;

	if (uw <= 0 || uh <= 0)
		while (uw <= 0 || uh <= 0)
		{
			std::cout << "Wrong units, please enter correct units\n";
			std::cout << "unit height ";
			std::cin >> uh;

			std::cout << "unit width ";
			std::cin >> uw;
		}

	block::set_unit_length(uh);
	block::set_unit_width(uw);

	integrated_circuit ob(lefFile, verilogFile);

	floorplaning f(ob, outputDefFile);

	std::srand(std::time(0));

	f.simulated_annealing();
	f.do_initial_palcement = false;
	f.simulated_annealing();
	
	f.simulated_annealing();
	ob.creat_def("output_2.def");

	std::cout << "\nrunning time is " << (double)(clock() - tStart) / CLOCKS_PER_SEC<<" seconds";
	return 0;
}