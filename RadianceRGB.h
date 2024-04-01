#pragma once
#include <string>

class RadianceRGB
{
public:
	double RGB[3];

public:
	RadianceRGB();
	RadianceRGB(double r, double g, double b);

	//��"30,50,30"����radiance�ַ���ת��ΪRGB������
	RadianceRGB(std::string radiance_str);

	//Radiance�ӷ�
	RadianceRGB operator +(const RadianceRGB& b);

	//Radiance����
	RadianceRGB operator *(double c)const;

	//����RGB[0]+RGB[1]+RGB[2]
	double sum();

	void print();
};

