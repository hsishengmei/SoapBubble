#pragma once
#include <map>

class IorCalculator {
public:
	IorCalculator(const char*);
	~IorCalculator() {}
	void output_result(const char*);
	void print_iorfile();
	void simulate_ior();
	float calc_Rs(float, float, int);
	float calc_Rp(float, float, int);
private:
	std::map<float, float> ior_file; // wavelength, n
	std::map<int, std::map<float, float[2]>> ior_sim; // ior_sim[theta_i][wavelength]=[R, T]
	int n_row;
};
