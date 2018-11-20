#include "IorCalculator.h"
#include <fstream>
#include <sstream>
#include <string>

IorCalculator::IorCalculator(const char* filename)
{
	std::string line;
	std::ifstream myfile(filename);
	if (myfile.is_open())
	{
		size_t n_row = 0;
		getline(myfile, line);
		while (getline(myfile, line))
		{
			n_row += 1;
			size_t pos = line.find(",");
			ior_file = new float[2];
			ior_file[n_row*2] = std::stof(line.substr(0, pos));
			ior_file[n_row*2+1] = std::stof(line.substr(pos+1));
		}
		myfile.close();
	}
}

IorCalculator::~IorCalculator()
{
	delete[] ior_file;
	delete[] ior_output;
}

void IorCalculator::outputResult(const char * filename)
{
	std::ofstream myfile(filename);
	if (myfile.is_open())
	{
		std::stringstream ss;

		for (int i = 0; i < sizeof(ior_file)/sizeof(float); i=i+2)
		{
			ss << ior_file[i] << "," << ior_file[i + 1] << "\n";
		}
		std::string mystring = ss.str();

		myfile << mystring;

		myfile.close();
	}
}
