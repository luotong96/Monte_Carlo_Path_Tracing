#pragma once
#include <string>
#include <map>
#include <vector>
#include "pugixml.hpp"
#include "RadianceRGB.h"
#include "Myobj.h"
#include <iostream>
//��Դ������
struct lightTriangle
{
	//��Դ����������shape�����
	size_t s;
	//��Դ�����ε����
	size_t f;
	//��Դ�����ε����
	double area;
	lightTriangle(size_t s, size_t f, double area)
	{
		this->s = s;
		this->f = f;
		this->area = area;
	}
};

//����select_a_point_from_lights_spherical_triangle(),�洢������̵��м����
struct projectedSphericalTriangle
{
	//�����������μ���Analytic Methods for simulated light transport�� Arvo 1995��5.2.1��5.2.2��
	//���߲ο���ʵ����ĵ�
	vec A, B, C;
	double alpha;
	double beta;
	double gamma;
	double a, b, c;
	//���������ε����
	double sA;


	//��Ӧ��Դ����������shape���
	size_t s;
	//��Ӧ��Դ�����ε����
	size_t f;

	//��Ӧ��Դ�ķ�������
	RadianceRGB L;

	projectedSphericalTriangle(vec A, vec B, vec C, double alpha, double beta, double gamma, double a, double b, double c, double sA, size_t s, size_t f, RadianceRGB L)
	{
		this->A = A;
		this->B = B;
		this->C = C;
		this->alpha = alpha;
		this->beta = beta;
		this->gamma = gamma;
		this->a = a;
		this->b = b;
		this->c = c;
		this->sA = sA;
		this->s = s;
		this->f = f;
		this->L = L;
	}
};

//�������ù�Դ��
struct sampledLightPoint
{
	//��Դ����������shape�����
	size_t s;
	//��Դ�����ε����
	size_t f;
	//�õ�����,Ӧ�����������ι�Դ�������ꡣ
	vec coord;
	//�õ��������
	RadianceRGB I;

	//�����õ�ĸ����ܶ�
	double prob;

	sampledLightPoint(size_t s, size_t f, vec coord, RadianceRGB I, double prob)
	{
		this->s = s;
		this->f = f;
		this->coord = coord;
		this->I = I;
		this->prob = prob;
	}
	void print()
	{
		std::cout << s << "|" << f << "|";
		coord.print();
		std::cout << "|";
		I.print();
		std::cout << "|" << prob << std::endl;
	}
};

class Mylight
{
	const double eps = 1e-8;
public:
	//��ȡ��xml�ļ�
	std::string inputfile;
	//��Դmtlname->RadianceRGB��ӳ��
	std::map<std::string, RadianceRGB> lightsRadiance;
	//��Դmtlname->light_triangle��ӳ�䡣�����ռ��洢���й�Դ������Ƭ����Ϣ��
	std::map<std::string, std::vector<lightTriangle> > lightsTriangles;

	//������Ƭ�Ƿ��ǹ�Դ
	std::map<triangle, RadianceRGB > islight;


//�洢Spherical triangle sampling�ĳ�ʼ����
	std::vector<projectedSphericalTriangle> sphericalTriangles;
	//����ѡ��ĳһ�������ι�Դ��Ȩ�أ��������ڵ�ǰ���ͶӰ���*Radiance
	std::vector<double> weights;
	double weights_sum;
	//�洢ĳ����Դ�������ڵ�ǰ�����ͶӰ������������sphericalTriangles�����е������±ꡣ
	std::map<triangle, size_t> indiceMap;

public:
	Mylight(std::string inputfile);
	//��xml�ж�ȡ��Դradiance����
	void read();
	//�����ռ����й�Դ������Ƭ����Ϣ��
	void gather_light_triangles(const tinyobj::ObjReader& reader);

	//�����й�Դ�о��Ȳ���һ���㣬���ص������vec,���������ܶ�double
	sampledLightPoint select_a_point_from_lights(Myobj& myobj);

	//�ӹ�Դ����ǰ��x1���ڰ������ͶӰ�������Ͼ��Ȳ���
	sampledLightPoint select_a_point_from_lights_spherical_triangle(vec x1,vec n,Myobj& myobj);


//�ӹ�Դ����ǰ��x1���ڰ������ͶӰ�������Ͼ��Ȳ������ֽ׶ΰ汾��
	//׼������
	void prepared_for_lights_spherical_triangle_sampling(vec x1, vec n, Myobj& myobj);
	//��ʽ�ӹ�Դsampleһ����
	sampledLightPoint lights_spherical_triangle_sampling(vec x1, vec n, Myobj& myobj);
	//������triangle������Դ�������в���һ��ĸ����ܶȡ�
	double eval_spherical_triangle_sampling_pdf(triangle tri, Myobj& myobj);
};

