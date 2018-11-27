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

void IorCalculator::init_ior(const std::string filename)
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
	// float p1 = n1*angle_cos(theta_i);
	// float p2 = n2*angle_cos(theta_t);
	// return (p1-p2)/(p1+p2);
	if (theta_i == 0) return (n1-n2) / (n1+n2);
	return -1*angle_sin(theta_i-theta_t) / angle_sin(theta_i+theta_t);
}

float IorCalculator::calc_rp(const float n1, const float n2, const float theta_i, const float theta_t)
{
	// float p1 = n2*angle_cos(theta_i);
	// float p2 = n1*angle_cos(theta_t);
	// return (p1-p2)/(p1+p2);
	if (theta_i == 0) return (n1-n2) / (n1+n2);
	return angle_tan(theta_i-theta_t) / angle_tan(theta_i+theta_t);
}

void IorCalculator::output_result()
{
	std::string s1 = "incident_angle.txt";
	std::string s2 = "wavelength.txt";
	std::string s3 = "reflectance.txt";
	std::string s4 = "transmittance.txt";

	std::ofstream f1(s1);
	std::ofstream f2(s2);
	std::ofstream f3(s3);
	std::ofstream f4(s4);

	if (f1.is_open() && f2.is_open() && f3.is_open() && f4.is_open())
	{
		std::stringstream ss1;
		std::stringstream ss2;
		std::stringstream ss3;
		std::stringstream ss4;

		for (std::map<int, std::map<float, float[2]>>::iterator it=ior_sim.begin(); it!=ior_sim.end(); ++it)
		{
			for (std::map<float, float[2]>::iterator it2=it->second.begin(); it2!=it->second.end(); ++it2)
			{
				ss1 << it->first << "\n";
				ss2 << it2->first << "\n";
				ss3 << it2->second[0] << "\n";
				ss4 << it2->second[1] << "\n";
			}
		}

		std::string o1 = ss1.str();
		std::string o2 = ss2.str();
		std::string o3 = ss3.str();
		std::string o4 = ss4.str();

		f1 << o1;
		f2 << o2;
		f3 << o3;
		f4 << o4;

		f1.close();
		f2.close();
		f3.close();
		f4.close();
	}
}
