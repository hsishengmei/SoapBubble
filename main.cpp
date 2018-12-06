#include "IorCalculator.h"
#include <string>
#include <cstring>
#include <iostream>

int main(int argc, char ** argv)
{
	IorCalculator IorCalc("water_new.csv");
	// IorCalculator IorCalc;
	// float n = 1.5;
	float thickness = std::stof(argv[1]); //nm
	IorCalc.init_thickness(thickness);
	IorCalc.simulate_bubble();
	IorCalc.output_result();
	IorCalc.write_log();
	return 0;
}
