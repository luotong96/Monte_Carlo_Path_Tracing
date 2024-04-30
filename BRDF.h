#pragma once
#include "vec.h"

//���߷���Ĳ������
struct sampledRay
{
	//���򣬵�λ����
	vec dir;
	//�����ܶ�
	double pdf;
	sampledRay(vec dir, double pdf)
	{
		this->dir = dir;
		this->pdf = pdf;
	}
};
class BRDF
{
public:
	double RGB[3];
public:
	BRDF(double a, double b, double c);
	static BRDF get_brdf_phong(vec n, vec wi,vec wr,vec diffuse, vec specular, double shiness);
	static sampledRay sample_from_phong(vec n, vec wr, vec diffuse, vec specular, double shiness);
	static double eval_sample_from_phong_pdf(vec n, vec wi, vec wr, vec diffuse, vec specular, double shiness);
};

