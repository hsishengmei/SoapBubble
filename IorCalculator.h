#pragma once

class IorCalculator {
public:
	IorCalculator(const char*);
	~IorCalculator();
	void outputResult(const char*);
private:
	float *ior_file;
	float *ior_output;
};