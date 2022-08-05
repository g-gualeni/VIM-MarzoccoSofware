#pragma once
#include <vector>

class Math
{
public: 
	Math();
	~Math();

	static float takeMax(std::vector<float> vector);
	static float takeMin(std::vector<float> vector);
	static int index(double chiave, std::vector<float> vector);
	
};