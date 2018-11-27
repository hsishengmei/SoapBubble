#include "IorCalculator.h"
#include <string>
#include <cstring>
#include <iostream>

int main(int argc, char ** argv)
{
	// IorCalculator IorCalc("C2H5OH.csv");
	IorCalculator IorCalc;
	float n = 1.33;
	float thickness = 500; //nm
	if (argc == 3)
	{
		if (strcmp(argv[1], "-f") == 0)
		{
			std::cout << "init from file\n";
			IorCalc.init_ior(argv[2]);
		}
		if (strcmp(argv[1], "-c") == 0)
		{
			std::cout << "init from constant\n";
			IorCalc.init_ior(std::stof(argv[2]));
		}
	}
	else
	{
		IorCalc.init_ior(n);
	}

	IorCalc.init_thickness(thickness);
	IorCalc.simulate_bubble();
	IorCalc.output_result("output2.csv");
	return 0;
}
