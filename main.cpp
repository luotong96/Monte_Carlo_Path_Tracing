#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT

#include "Myobj.h"

#include <random>
#include "Mylight.h"
#include <iostream>


const std::string root_file_path = ".\\Debug\\veach-mis\\veach-mis";
//const std::string root_file_path = ".\\Debug\\cornell-box\\cornell-box";
//const std::string root_file_path = ".\\Debug\\bathroom\\bathroom";

Myobj veach(root_file_path + ".obj");
Mylight lights(root_file_path + ".xml");

inline vec linear_interpolation(std::array<vec, 3> vlist, double a, double b, double c)
{
	return vlist[0] * a + vlist[1] * b + vlist[2] * c;
}

//Path tracing，由摄像头发射光线。
//从ro点沿着rd方向射出光线，到达ro点的累积概率是px。返回值是从光源射入当前点ro的radiance。
//递归向下求概率，回溯算radiance。
RadianceRGB shoot(vec ro, vec rd, double &px)
{
	//求交
	intersec_result rs = veach.closet_ray_intersect(ro, rd);
	
	vec nro = linear_interpolation(veach.get_vertexes_of_facet(rs.s, rs.f), 1.0 - rs.bgt[0] - rs.bgt[1], rs.bgt[0], rs.bgt[1]);
	vec normal = linear_interpolation(veach.get_normals_of_facet(rs.s, rs.f), 1.0 - rs.bgt[0] - rs.bgt[1], rs.bgt[0], rs.bgt[1]);
	
	tinyobj::material_t mtl = veach.reader.GetMaterials().at(veach.reader.GetShapes().at(rs.s).mesh.material_ids[rs.f]);

	//俄罗斯轮盘,以q的概率继续发射光线。q取kd（3项）和ks（3项）的6项平均值。
	double q = (mtl.diffuse[0] + mtl.diffuse[1] + mtl.diffuse[2] + mtl.specular[0] + mtl.specular[1] + mtl.specular[2]) / 6;
	q = fmin(1, q);
	
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution(0.0, 1.0);
	double ksi = distribution(generator);
	//mtl.name
	//停止继续发射光线。
	if (ksi >= q)
	{
		
	}
	//采样新的方向，射出光线
	return RadianceRGB(0,0,0);
}
int main()
{
	
	veach.read();
	lights.read();
	lights.gather_light_triangles(veach.reader);
	sampledLightPoint a = lights.select_a_point_from_lights(veach);
	a.print();
	
	return 0;
}