#include "IorCalculator.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <complex>

void IorCalculator::init_ior(const float f)
{
	ior_fixed = f;
	std::cout << "successful init: ior_fixed=" << f << "\n";
}

void IorCalculator::init_ior(const char* filename)
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
			if (wavelength >= min_wl && wavelength <= max_wl)
			{
				ior_file[wavelength] = ior;
			}
			n_row += 1;
		}

		print_iorfile();
		myfile.close();
	}
	std::cout << "successful init: ior_file\n";
}

void IorCalculator::init_thickness(const float f)
{
	thickness = f;
	std::cout << "successful init: thickness=" << f << "\n";
}

void IorCalculator::print_iorfile()
{
	for (std::map<float, float>::iterator it=ior_file.begin(); it!=ior_file.end(); ++it)
	{
		std::cout << it->first << ":" << it->second << '\n';
	}
}

void IorCalculator::simulate_bubble()
{
	for (int wl=min_wl; wl<=max_wl; wl+=10)
	{
		float n1 = 1.0;
		float n2 = ior_fixed;
		if (n2==0) n2 = ior_file[wl];

		float crit_angle = 90;
		if (n2 < n1)
		{
			crit_angle = angle_asin(n2/n1);
		}

		for (int theta_i = 90; theta_i >= 0; --theta_i)
		{
			float Reff = 1;

			if (theta_i <= crit_angle)
			{
				float theta_t = angle_asin(n1*angle_sin(theta_i)/n2); // n1sin1=n2sin2
				std::cout << "theta_i:" << theta_i << ", theta_t:" << theta_t << " wl:" << wl << '\n';
				Reff = calc_recur_R(n1, n2, theta_i, theta_t, wl, thickness);
			}

			// no absorption
			float Teff = 1 - Reff;
			ior_sim[theta_i][wl][0] = Reff;
			ior_sim[theta_i][wl][1] = Teff;
		}

	}
}

float IorCalculator::calc_recur_R(const float n1, const float n2, const float theta_i, const float theta_t, const int wl, const int d)
{
	float rs12 = calc_rs(n1, n2, theta_i, theta_t);
	float rp12 = calc_rp(n1, n2, theta_i, theta_t);
	float rs21 = calc_rs(n2, n1, theta_t, theta_i);
	float rp21 = calc_rp(n2, n1, theta_t, theta_i);

	std::cout << "rs12:" << rs12 << ", rp12:" << rp12 << ", rs21:" << rs21 << ", rp21:" << rp21 << "\n";

	float ts12 = rs12+1;
	float tp12 = (rp12+1)*n1/n2;
	float ts21 = rs21+1;
	float tp21 = (rp21+1)*n2/n1;

	std::cout << "ts12:" << ts12 << ", tp12:" << tp12 << ", ts21:" << ts21 << ", tp21:" << tp21 << "\n";

	float phi = 2*2*(PI/wl)*n2*d*angle_cos(theta_t);
	std::complex<float> phase_diff = std::exp(-1*phi*std::complex<float>(0,1));
	std::complex<float> phase_diff_half = std::exp(-1*(phi/2)*std::complex<float>(0,1));
	std::complex<float> rs_recur = (rs12 + rs21*phase_diff) / (std::complex<float>(1,0)+rs12*rs21*phase_diff);
	std::complex<float> rp_recur = (rp12 + rp21*phase_diff) / (std::complex<float>(1,0)+rp12*rp21*phase_diff);
	std::complex<float> ts_recur = (ts12*ts21*phase_diff_half) / (std::complex<float>(1,0)+rs12*rs21*phase_diff);
	std::complex<float> tp_recur = (tp12*tp21*phase_diff_half) / (std::complex<float>(1,0)+rp12*rp21*phase_diff);

	std::cout << "rs:" << rs_recur << ", rp:" << rp_recur << ", ts:" << ts_recur << ", tp:" << tp_recur << "\n";

	float abs_s = abs(rs_recur);
	float abs_p = abs(rp_recur);
	float Reff = (abs_s*abs_s+abs_p*abs_p)/2;

	float abs_ts = abs(ts_recur);
	float abs_tp = abs(tp_recur);
	float Teff = (abs_ts*abs_ts+abs_tp*abs_tp)/2;


	std::cout << "|rs|:" << abs_s << ", |rp|:" << abs_p << ", Reff:" << Reff << "\n";
	std::cout << "|ts|:" << abs_ts << ", |tp|:" << abs_tp << ", Teff:" << Teff << "\n\n";

	return Reff;
}

float IorCalculator::calc_rs(const float n1, const float n2, const float theta_i, const float theta_t)
{
	float p1 = n1*angle_cos(theta_i);
	float p2 = n2*angle_cos(theta_t);
	return (p1-p2)/(p1+p2);
}

float IorCalculator::calc_rp(const float n1, const float n2, const float theta_i, const float theta_t)
{
	float p1 = n2*angle_cos(theta_i);
	float p2 = n1*angle_cos(theta_t);
	return (p1-p2)/(p1+p2);
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
