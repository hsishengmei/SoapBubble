#pragma once
#include <map>
#include <math.h>

class IorCalculator {
public:
	IorCalculator() {}
	IorCalculator(const float f) { init_ior(f); }
	IorCalculator(const char* c) { init_ior(c); }
	~IorCalculator() {}
	void init_ior(const float);
	void init_ior(const char*);
	void init_thickness(const float);
	void output_result(const char*);
	void print_iorfile();
	void simulate_bubble();
	float calc_recur_R(const float, const float, const float, const float, const int, const int);
	float calc_rs(const float, const float, const float, const float);
	float calc_rp(const float, const float, const float, const float);

	void test_trigometry(const float);

private:
	std::map<float, float> ior_file; // wavelength, n
	std::map<int, std::map<float, float[2]>> ior_sim; // ior_sim[theta_i][wavelength]=[R, T]
	int n_row;
	float ior_fixed = 0;
	float thickness;
	const int min_wl = 400;
	const int max_wl = 700;

	float angle_sin(float t) { return sin(a2r(t)); }
	float angle_cos(float t) { return cos(a2r(t)); }
	float angle_asin(float n) { return r2a(asin(n)); }
	float angle_acos(float n) { return r2a(acos(n)); }
	float a2r(float a) { return a*PI/180; } // angle to rad
	float r2a(float r) { return r*180/PI; } // angle to rad
	float PI=M_PI; //3.14159265;
};
