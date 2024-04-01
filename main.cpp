#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT

#include "Myobj.h"

#include <random>
#include "Mylight.h"
#include <iostream>
#include <numbers>
#include <chrono>

const std::string root_file_path = ".\\Debug\\veach-mis\\veach-mis";
//const std::string root_file_path = ".\\Debug\\cornell-box\\cornell-box";
//const std::string root_file_path = ".\\Debug\\bathroom\\bathroom";

Myobj veach(root_file_path + ".obj");
Mylight lights(root_file_path + ".xml");

inline vec linear_interpolation(std::array<vec, 3> vlist, double a, double b, double c)
{
	return vlist[0] * a + vlist[1] * b + vlist[2] * c;
}


inline double inverse_F_theta(double x, double y, double n, double ksi,double error)
{
	//�˵�������⴦��
	if (ksi <= 0)
		return 0;
	if (ksi >= 1)
		return 0.5 * std::numbers::pi;
	//ţ�ٵ�������F(theta)��������
	double theta = 0.25 * std::numbers::pi;
	while (1)
	{
		double Fx = 1 - ksi - x * (n + 1) / (x * (n + 1) + y) * cos(theta) - y / (x * (n + 1) + y) * pow(cos(theta), n + 1);
		if (fabs(Fx) < error)
		{
			break;
		}
		double fx = (n + 1) * sin(theta) / (x * (n + 1) + y) * (x + y * pow(cos(theta), n));
		double fd =  Fx / fx;
		//double fd = (1 - ksi) * (x * (n + 1) + y) - cos(theta) * (x * (n + 1) + y * pow(cos(theta), n)) / ((n + 1) * sin(theta) * (x + y * pow(cos(theta), n)));
		theta -= fd;
	}

	return theta;
}

//Path tracing��������ͷ������ߡ�
//wi* x        .x2 wo*
//     .      .
//      .   .
//  w*   .x1
//��x������w����������ߣ�wΪ��λ����������x�㲢��w����������ۻ������ܶ���px������ֵ�Ǵӹ�Դ����x��radiance��
//�ݹ���������ʣ�������radiance��
RadianceRGB shoot(vec x, vec w, double &px)
{
	//��
	intersec_result rs = veach.closet_ray_intersect(x, w);
	if (rs.isIntersec == false)
		return RadianceRGB(0,0,0);

	vec x1 = linear_interpolation(veach.get_vertexes_of_facet(rs.s, rs.f), 1.0 - rs.beta - rs.gamma, rs.beta, rs.gamma);
	vec N = linear_interpolation(veach.get_normals_of_facet(rs.s, rs.f), 1.0 - rs.beta - rs.gamma, rs.beta, rs.gamma);
	
	tinyobj::material_t mtl = veach.reader.GetMaterials().at(veach.reader.GetShapes().at(rs.s).mesh.material_ids[rs.f]);
	
	//���ڻ��ݼ���radiance�ı���
	RadianceRGB Ii;
	vec V = w * (-1);
	vec L;
	vec H;

	//����˹����,��q�ĸ��ʼ���������ߡ�qȡkd��3���ks��3���6��ƽ��ֵ��
	double q = (mtl.diffuse[0] + mtl.diffuse[1] + mtl.diffuse[2] + mtl.specular[0] + mtl.specular[1] + mtl.specular[2]) / 6;
	q = fmin(1, q);
	
	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

	std::default_random_engine generator(seed1);
	std::uniform_real_distribution<double> distribution(0.0, 1.0);
	double ksi = distribution(generator);

	//ֹͣ����������ߡ�
	if (ksi >= q)
	{
		sampledLightPoint lightpoint = lights.select_a_point_from_lights(veach);
		px *= lightpoint.prob;

		vec wo = (lightpoint.coord - x1).normalized();
		L = wo;
		H = (L + V).normalized();

		intersec_result rs1 = veach.closet_ray_intersect(x1, wo);
		//�����Դ����x1֮�����ڵ������·��Ч��radiance����Ϊ0
		if((rs1.s != lightpoint.s) || (rs1.f != lightpoint.f))
		{
			Ii = RadianceRGB(0, 0, 0);
		}
		else
		{
			Ii = lightpoint.I;
		}
	}
	else
	{
		//��Blin-phongģ�͵�brdf��sample�µĳ��䷽��
		//�ݹ����µ�ʱ�򣬲���֪����ʲô��ɫ�Ĺ⣬���ֱ��ʹ��kd����ά�ȵĺ͡�
		//����ʹ��-w��Ϊ���������ΪĿǰ���ڴ�����ͷ��ʼ���ݹ����µĽ׶Ρ�ʹ��brdf����������µĳ��䷽��Ŀǰ�Ĺ�·����ݼ���Radiance�Ĺ�·���෴�ġ�
		vec l = w * -1;
		double x = ((double)mtl.diffuse[0] + mtl.diffuse[1] + mtl.diffuse[2]) * N.dot_product(l);
		double y = (double)mtl.specular[0] + mtl.specular[1] + mtl.specular[2];
		double n = mtl.shininess;
		
		double ksi1 = distribution(generator);

		double theta = inverse_F_theta(x, y, n, ksi1, 1e-9);

		double ksi2 = distribution(generator);
		double phi = 2 * std::numbers::pi * ksi2;
		//h��l��v�İ�ǵ�λ����
		vec h(cos(phi) * sin(theta), sin(phi) * sin(theta), cos(theta));
		//�����������������ͬһƽ������Ϊ���ཻ�����Ա�֤�������hһ����Ϊ0
		vec v = h * (2 * h.dot_product(l)) - l;
		
		//�۳��·���ĸ����ܶ�
		double prob = (x + y * pow(cos(theta), n)) * sin(theta) / (2 * std::numbers::pi * (x + y / (n + 1.0)));
		px *= prob;

		//����Radiance,��Russian Roulette����Ҫ��1/q�������ص�����
		Ii = shoot(x1, v, px) * (1.0 / q);
		
		//��·��ת�������õ���v�����Ϊ����x1��x�������������е�L����
		L = v;
		H = (L + V).normalized();
	}
	
	//��ʼʹ��blin-phong����Radiance
	RadianceRGB kd = RadianceRGB(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]);
	RadianceRGB ks = RadianceRGB(mtl.specular[0], mtl.specular[1], mtl.specular[2]);
	double ns = mtl.shininess;
	RadianceRGB Ie = Ii * (kd * N.dot_product(L) + ks * pow(H.dot_product(N), ns));
	if (Ie.RGB[0] < 0 || Ie.RGB[1] < 0 || Ie.RGB[2] < 0)
	{
		;
	}
	return Ie;
}
int main()
{

	veach.read();
	lights.read();
	lights.gather_light_triangles(veach.reader);
	for (int i = 0; i < 1000; i++)
	{
		std::cout << "Radiance: ";
		double p = 1;
		shoot(vec(28.2792, 5.2, 1.23612e-06), (vec(0.0, 2.8, 0.0)- vec(28.2792, 5.2, 1.23612e-06)).normalized(), p).print();
		std::cout << " p:" << p << std::endl;
	}

	return 0;
}