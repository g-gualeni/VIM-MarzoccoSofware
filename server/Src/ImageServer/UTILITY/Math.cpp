#include "Math.h"
#include <iostream>
#include <algorithm>

Math::Math()
{

}

Math::~Math()
{

}

float Math::takeMax(std::vector<float> Z_values)
{

	float maxValue = FLT_MIN;
	
	for (int i = 0; i < Z_values.size(); i++)
	{
		if (maxValue <= Z_values.at(i))
				maxValue = Z_values.at(i);
		
	}
	if (maxValue == FLT_MIN)
		maxValue = 0;
	return maxValue;
}

float Math::takeMin(std::vector<float> Z_values)
{
	float minValue = FLT_MAX;
	for (int i = 0; i < Z_values.size(); i++)
	{
		if (minValue > Z_values.at(i))
				minValue = Z_values.at(i);
		
	}
	if (minValue == FLT_MAX)
		minValue = 0;
	return minValue;
}

int Math::index(double chiave, std::vector<float> vector)
{
	for (int i = 0; i < vector.size(); i++)
	{
		if (vector.at(i) == chiave)
		{
			return i;
		}
	}
	return -1;
}






