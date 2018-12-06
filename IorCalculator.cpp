#include "IorCalculator.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <complex>

void IorCalculator::init_ior(const float f)
{
	ior_fixed = f;
	ss_log << "successful init: ior_fixed=" << f << "\n";
}

void IorCalculator::init_ior(const std::string filename)
{
	std::string line;
	std::ifstream myfile(filename);
	if (myfile.is_open())
	{
		n_row = 0;
		//getline(myfile, line);
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
	ss_log << "successful init: ior_file\n";
}

void IorCalculator::init_thickness(const float f)
{
	thickness = f;
	ss_log << "successful init: thickness=" << f << "\n";
}

void IorCalculator::print_iorfile()
{
	for (std::map<float, float>::iterator it=ior_file.begin(); it!=ior_file.end(); ++it)
	{
		ss_log << it->first << ":" << it->second << '\n';
	}
}

void IorCalculator::write_log()
{

		std::ofstream f_log("log.txt");

		if (f_log.is_open())
		{
			std::string str_log = ss_log.str();

			f_log << str_log;

			f_log.close();
		}
}

void IorCalculator::simulate_bubble()
{
	float n1 = 1;
	float n3 = 1;
	for (int wl=min_wl; wl<=max_wl; wl+=10)
	{
		float n2 = ior_file[wl];
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
				ss_log << "theta_i:" << theta_i << ", theta_t:" << theta_t << " wl:" << wl << '\n';
				Reff = calc_recur_R(n1, n2, n3, theta_i, theta_t, theta_i, wl, thickness);
			}

			// no absorption
			float Teff = 1 - Reff;
			ior_sim[theta_i][wl][0] = Reff;
			ior_sim[theta_i][wl][1] = Teff;
		}

	}
}

float IorCalculator::calc_recur_R(const float n1, const float n2, const float n3, const float theta_1, const float theta_2, const float theta_3, const int wl, const int d)
{
	// bugs to be fixed:
	// when caluclating theta_3 at crit_angle, value of angle_asin can be "nan"

	float rs1 = calc_rs(n1, n2, theta_1, theta_2);
	float rp1 = calc_rp(n1, n2, theta_1, theta_2);
	float rs2 = calc_rs(n2, n3, theta_2, theta_3);
	float rp2 = calc_rp(n2, n3, theta_2, theta_3);

	ss_log << "rs1:" << rs1 << ", rp1:" << rp1 << ", rs2:" << rs2 << ", rp2:" << rp2 << "\n";

	float ts1 = rs1+1;
	float tp1 = (rp1+1)*n1/n2;
	float ts2 = rs2+1;
	float tp2 = (rp2+1)*n2/n1;

	ss_log << "ts1:" << ts1 << ", tp1:" << tp1 << ", ts2:" << ts2 << ", tp2:" << tp2 << "\n";

	float phi = 2*2*(PI/wl)*n2*d*angle_cos(theta_2);
	std::complex<float> one(1,0);
	std::complex<float> i(0,1);
	std::complex<float> phase_diff = std::exp(-1*phi*i);
	std::complex<float> phase_diff_half = std::exp(-1*(phi/2)*i);
	std::complex<float> rs_recur = (rs1*one + rs2*phase_diff) / (one+rs1*rs2*phase_diff);
	std::complex<float> rp_recur = (rp1*one + rp2*phase_diff) / (one+rp1*rp2*phase_diff);
	std::complex<float> ts_recur = (ts1*ts2*phase_diff_half) / (one+rs1*rs2*phase_diff);
	std::complex<float> tp_recur = (tp1*tp2*phase_diff_half) / (one+rp1*rp2*phase_diff);

	ss_log << "rs:" << rs_recur << ", rp:" << rp_recur << ", ts:" << ts_recur << ", tp:" << tp_recur << "\n";

	float abs_s = abs(rs_recur);
	float abs_p = abs(rp_recur);
	float Reff = (abs_s*abs_s+abs_p*abs_p)/2;

	float abs_ts = abs(ts_recur);
	float abs_tp = abs(tp_recur);
	float Teff = (abs_ts*abs_ts+abs_tp*abs_tp)/2;


	ss_log << "|rs|:" << abs_s << ", |rp|:" << abs_p << ", Reff:" << Reff << "\n";
	ss_log << "|ts|:" << abs_ts << ", |tp|:" << abs_tp << ", Teff:" << Teff << "\n\n";

	return Reff;
}

float IorCalculator::calc_rs(const float n1, const float n2, const float theta_i, const float theta_t)
{
	if (theta_i == 90) return -1;
	if (theta_t == 90) return 1;
	float p1 = n1*angle_cos(theta_i);
	float p2 = n2*angle_cos(theta_t);
	return (p1-p2)/(p1+p2);
	// if (theta_i == 0) return (n1-n2) / (n1+n2);
	// return -1*angle_sin(theta_i-theta_t) / angle_sin(theta_i+theta_t);
}

float IorCalculator::calc_rp(const float n1, const float n2, const float theta_i, const float theta_t)
{
	if (theta_i == 90) return -1;
	if (theta_t == 90) return 1;
	float p1 = n2*angle_cos(theta_i);
	float p2 = n1*angle_cos(theta_t);
	return (p1-p2)/(p1+p2);
	// if (theta_i == 0) return (n1-n2) / (n1+n2);
	// return angle_tan(theta_i-theta_t) / angle_tan(theta_i+theta_t);
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
