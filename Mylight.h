#pragma once
#include <string>
#include <map>
#include <vector>
#include "pugixml.hpp"
#include "RadianceRGB.h"
#include "Myobj.h"
#include <iostream>
//光源三角形
struct lightTriangle
{
	//光源三角形所在shape的序号
	size_t s;
	//光源三角形的序号
	size_t f;
	//光源三角形的面积
	double area;
	lightTriangle(size_t s, size_t f, double area)
	{
		this->s = s;
		this->f = f;
		this->area = area;
	}
};

//采样所得光源点
struct sampledLightPoint
{
	//光源三角形所在shape的序号
	size_t s;
	//光源三角形的序号
	size_t f;
	//该点坐标
	vec coord;
	//该点辐射亮度
	RadianceRGB I;

	//采样该点的概率密度
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
	//光源mtlname->RadianceRGB的映射
	std::map<std::string, RadianceRGB> lightsRadiance;
	//光源mtlname->light_triangle的映射。用于收集存储所有光源三角面片的信息。
	std::map<std::string, std::vector<lightTriangle> > lightsTriangles;

public:
	Mylight(std::string inputfile);
	//从xml中读取光源radiance数据
	void read();
	//用于收集所有光源三角面片的信息。
	void gather_light_triangles(const tinyobj::ObjReader& reader);

	//从所有光源中采样一个点，返回点的坐标vec,采样概率密度double
	sampledLightPoint select_a_point_from_lights(const Myobj& myobj);
};

