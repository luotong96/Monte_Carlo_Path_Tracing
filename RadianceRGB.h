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

	//��"30,50,30"����radiance�ַ���ת��ΪRGB������
	RadianceRGB(std::string radiance_str);

	//Radiance�ӷ�
	RadianceRGB operator +(const RadianceRGB& b);

	//Radiance����
	RadianceRGB operator *(double c)const;

	//Radiance pairwise �˷�
	RadianceRGB operator *(const RadianceRGB& b)const;
	
	//Radiance * BRDF
	RadianceRGB operator *(const BRDF& b)const;

	//����RGB[0]+RGB[1]+RGB[2]
	double sum();

	void print();

	//Radiance ��RGB��ת��
	std::array<int, 3> tone_mapping(double maxRadiance, double gamma);
};

