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

//�������ù�Դ��
struct sampledLightPoint
{
	//��Դ����������shape�����
	size_t s;
	//��Դ�����ε����
	size_t f;
	//�õ�����
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

public:
	std::string inputfile;
	//��Դmtlname->RadianceRGB��ӳ��
	std::map<std::string, RadianceRGB> lightsRadiance;
	//��Դmtlname->light_triangle��ӳ�䡣�����ռ��洢���й�Դ������Ƭ����Ϣ��
	std::map<std::string, std::vector<lightTriangle> > lightsTriangles;

public:
	Mylight(std::string inputfile);
	//��xml�ж�ȡ��Դradiance����
	void read();
	//�����ռ����й�Դ������Ƭ����Ϣ��
	void gather_light_triangles(const tinyobj::ObjReader& reader);

	//�����й�Դ�в���һ���㣬���ص������vec,���������ܶ�double
	sampledLightPoint select_a_point_from_lights(const Myobj& myobj);
};

