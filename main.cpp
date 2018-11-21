#include "IorCalculator.h"

int main()
{
	IorCalculator IorCalc("CuZn.csv");
	IorCalc.simulate_ior();
	IorCalc.output_result("output.csv");
	return 0;
}
