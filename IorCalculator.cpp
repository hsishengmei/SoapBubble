#include "IorCalculator.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <math.h>
#define PI 3.14159265

IorCalculator::IorCalculator(const char* filename)
{
	std::string line;
	std::ifstream myfile(filename);
	if (myfile.is_open())
	{
		n_row = 0;
		getline(myfile, line);
		while (getline(myfile, line))
		{
			size_t pos = line.find(",");
			float wavelength = std::stof(line.substr(0, pos)) * 1000;
			float ior = std::stof(line.substr(pos+1));
			if (wavelength > 300 && wavelength < 900)
			{
				ior_file[wavelength] = ior;
			}
			n_row += 1;
		}

		print_iorfile();
		myfile.close();
	}
}

void IorCalculator::print_iorfile()
{
	for (std::map<float, float>::iterator it=ior_file.begin(); it!=ior_file.end(); ++it)
	{
		std::cout << it->first << " => " << it->second << '\n';
	}
}

void IorCalculator::simulate_ior()
{
	for (std::map<float, float>::iterator it = ior_file.begin(); it != ior_file.end(); ++it)
	{
		float n1 = it->second;
		float n2 = 1.0;

		for (int theta_i = 1; theta_i <= 90; ++theta_i)
		{
			float Rs = calc_Rs(n1, n2, theta_i);
			float Rp = calc_Rp(n1, n2, theta_i);
			float Reff = (Rs+Rp)/2;
			float Teff = 1 - Reff;
			std::cout << theta_i << "," << it->first << "," << Reff << "," << Teff << "\n";
			ior_sim[theta_i][it->first][0] = Reff;
			ior_sim[theta_i][it->first][1] = Teff;
		}

	}
}

float IorCalculator::calc_Rs(float n1, float n2, int theta_i)
{
	float rad_i = theta_i*PI/180;
	float p1 = n1*cos(rad_i);
	float p2 = n2*sqrt(1-(sin(rad_i)*n1/n2)*(sin(rad_i)*n1/n2));
	return ((p1-p2)/(p1+p2))*((p1-p2)/(p1+p2));
}

float IorCalculator::calc_Rp(float n1, float n2, int theta_i)
{
	float rad_i = theta_i*PI/180;
	float p1 = n1*sqrt(1-(sin(rad_i)*n1/n2)*(sin(rad_i)*n1/n2));
	float p2 = n2*cos(rad_i);
	return ((p1-p2)/(p1+p2))*((p1-p2)/(p1+p2));
}

void IorCalculator::output_result(const char * filename)
{
	std::ofstream myfile(filename);
	if (myfile.is_open())
	{
		std::stringstream ss;
		ss << "Incident angle,Wavelength,Reflectance,Transmittance\n";

		for (std::map<int, std::map<float, float[2]>>::iterator it=ior_sim.begin(); it!=ior_sim.end(); ++it)
		{
			for (std::map<float, float[2]>::iterator it2=it->second.begin(); it2!=it->second.end(); ++it2)
			{
				ss << it->first << "," << it2->first << ","
					 << it2->second[0] << "," << it2->second[1] << "\n";
			}
		}

		std::string mystring = ss.str();
		myfile << mystring;

		myfile.close();
	}
}
