#pragma once
#include <string>
#include <array>
#include "BRDF.h"
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

	//Radiance pairwise 乘法
	RadianceRGB operator *(const RadianceRGB& b)const;
	
	//Radiance * BRDF
	RadianceRGB operator *(const BRDF& b)const;

	//返回RGB[0]+RGB[1]+RGB[2]
	double sum();

	void print();

	//Radiance 到RGB的转换
	std::array<int, 3> tone_mapping(double maxRadiance, double gamma);
};

