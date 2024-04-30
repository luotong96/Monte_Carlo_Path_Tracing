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

//用于select_a_point_from_lights_spherical_triangle(),存储计算过程的中间参数
struct projectedSphericalTriangle
{
	//具体参数含义参见《Analytic Methods for simulated light transport》 Arvo 1995，5.2.1、5.2.2节
	//或者参考本实验的文档
	vec A, B, C;
	double alpha;
	double beta;
	double gamma;
	double a, b, c;
	//球面三角形的面积
	double sA;


	//对应光源三角形所在shape序号
	size_t s;
	//对应光源三角形的序号
	size_t f;

	//对应光源的辐射亮度
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

//采样所得光源点
struct sampledLightPoint
{
	//光源三角形所在shape的序号
	size_t s;
	//光源三角形的序号
	size_t f;
	//该点坐标,应当是在三角形光源处的坐标。
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
	const double eps = 1e-8;
public:
	//读取的xml文件
	std::string inputfile;
	//光源mtlname->RadianceRGB的映射
	std::map<std::string, RadianceRGB> lightsRadiance;
	//光源mtlname->light_triangle的映射。用于收集存储所有光源三角面片的信息。
	std::map<std::string, std::vector<lightTriangle> > lightsTriangles;

	//三角面片是否是光源
	std::map<triangle, RadianceRGB > islight;


//存储Spherical triangle sampling的初始数据
	std::vector<projectedSphericalTriangle> sphericalTriangles;
	//具体选择某一个三角形光源的权重，等于其在当前点的投影面积*Radiance
	std::vector<double> weights;
	double weights_sum;
	//存储某个光源三角形在当前半球的投影三角形数据在sphericalTriangles数组中的整数下标。
	std::map<triangle, size_t> indiceMap;

public:
	Mylight(std::string inputfile);
	//从xml中读取光源radiance数据
	void read();
	//用于收集所有光源三角面片的信息。
	void gather_light_triangles(const tinyobj::ObjReader& reader);

	//从所有光源中均匀采样一个点，返回点的坐标vec,采样概率密度double
	sampledLightPoint select_a_point_from_lights(Myobj& myobj);

	//从光源到当前点x1所在半球面的投影三角形上均匀采样
	sampledLightPoint select_a_point_from_lights_spherical_triangle(vec x1,vec n,Myobj& myobj);


//从光源到当前点x1所在半球面的投影三角形上均匀采样（分阶段版本）
	//准备计算
	void prepared_for_lights_spherical_triangle_sampling(vec x1, vec n, Myobj& myobj);
	//正式从光源sample一个点
	sampledLightPoint lights_spherical_triangle_sampling(vec x1, vec n, Myobj& myobj);
	//计算在triangle所属光源三角形中采样一点的概率密度。
	double eval_spherical_triangle_sampling_pdf(triangle tri, Myobj& myobj);
};

