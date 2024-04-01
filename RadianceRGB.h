#pragma once
#include <string>

class RadianceRGB
{
public:
	double RGB[3];

public:
	RadianceRGB();
	RadianceRGB(double r, double g, double b);

	//将"30,50,30"这种radiance字符串转换为RGB向量。
	RadianceRGB(std::string radiance_str);

	//Radiance加法
	RadianceRGB operator +(const RadianceRGB& b);

	//Radiance数乘
	RadianceRGB operator *(double c)const;

	//返回RGB[0]+RGB[1]+RGB[2]
	double sum();

	void print();
};

