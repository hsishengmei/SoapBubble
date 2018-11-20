#include "IorCalculator.h"

int main() 
{
	IorCalculator IorCalc("CuZn.csv");
	IorCalc.outputResult("output.csv");
	return 0;
}