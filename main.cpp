#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
//#define TINYOBJLOADER_USE_MAPBOX_EARCUT

#include "Myobj.h"

#include <random>
#include "Mylight.h"
#include <iostream>
#include <numbers>
#include <chrono>
#include "matrix3d.h"
#include <graphics.h>		// ���� EasyX ��ͼ��ͷ�ļ�
#ifdef RGB
#undef RGB
#endif


const std::string root_file_path = ".\\Debug\\veach-mis\\veach-mis";
//const std::string root_file_path = ".\\Debug\\cornell-box\\cornell-box";
//const std::string root_file_path = ".\\Debug\\bathroom\\bathroom";

Myobj veach(root_file_path + ".obj");
Mylight lights(root_file_path + ".xml");

inline vec linear_interpolation(const std::array<vec, 3>& vlist, double a, double b, double c)
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

inline double cal_theta0(double a1,double a2,double a3,double phi)
{
	const double eps = 1e-8;
	//����һ��a1 * cos(phi) + a2 * sin(phi) = 0����������ڣ������������ƽ�淨��ֱ���ɻ�ͼ����
	if (fabs(a1) < eps && fabs(a2) < eps)
		return 0.5 * std::numbers::pi;
	return atan(-a3 / (a1 * cos(phi) + a2 * sin(phi)));
}

//integrand_func��������
inline double func(double phi, double x, double y, double n,double a1, double a2, double a3)
{
	//double theta0 = atan(-a3 / (a1 * cos(phi) + a2 * sin(phi)));
	double theta0 = cal_theta0(a1, a2, a3, phi);
	return x * cos(theta0) + y * pow(cos(theta0), n + 1) / (n + 1);
}

//����Rejection���������������ܶȵĳ˻�����M��
inline double cal_M(double c2, double x, double y, double n, double a1, double a2, double a3)
{
	int interval_n = 100;
	double h = std::numbers::pi/ interval_n;
	double sum = 0;
	for (int i = 0; i < interval_n; i++)
	{
		sum += h * (func(i * h, x, y, n, a1, a2, a3) + 4 * func((i + 0.5) * h, x, y, n, a1, a2, a3) + func((i + 1) * h, x, y, n, a1, a2, a3)) / 6;
	}
	
	return 1.0 / (1 - c2 * sum);
}

//Path tracing��������ͷ������ߡ�
//wi* x        .x2 wo*
//     .      .
//      .   .
//  w*   .x1
//��x������w����������ߣ�wΪ��λ����������x�㲢��w����������ۻ������ܶ���px������ֵ�Ǵӹ�Դ����x��radiance��
//�ݹ���������ʣ�������radiance��
RadianceRGB shoot(vec x, vec w, double &px, int& step)
{
	step++;
	//��
	intersec_result rs = veach.closet_ray_intersect(x, w);
	if (rs.isIntersec == false)
		return RadianceRGB(0, 0, 0);

	vec x1 = linear_interpolation(veach.get_vertexes_of_facet(rs.s, rs.f), 1.0 - rs.beta - rs.gamma, rs.beta, rs.gamma);
	vec N = linear_interpolation(veach.get_normals_of_facet(rs.s, rs.f), 1.0 - rs.beta - rs.gamma, rs.beta, rs.gamma).normalized();
	//vec N = veach.get_unique_normal_of_facet(rs.s, rs.f);
	if (N.dot_product(w * -1) < 0)
	{
		double tmp = N.dot_product(w * -1);
		//printf("95\n");
		return RadianceRGB(0, 0, 0);
	}
	

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

		//�����Դ�ڵ�ǰfacet�ı��棬���·��Ч��
		if (L.dot_product(N) < 0)
		{
			Ii = RadianceRGB(0, 0, 0);
		}
		else {
			intersec_result rs1 = veach.closet_ray_intersect(x1, wo);
			//�����Դ����x1֮�����ڵ������·��Ч��radiance����Ϊ0
			if ((rs1.s != lightpoint.s) || (rs1.f != lightpoint.f))
			{
				Ii = RadianceRGB(0, 0, 0);
			}
			else
			{
				Ii = lightpoint.I;
			}
		}
	}
	else
	{
		//��Phongģ�͵�brdf��sample�µĳ��䷽��
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


		//specular�����У�N�뷴���r���ڻ�������N�������l���ڻ���
		double NR = N.dot_product(l);
		//sqNR��Ϊ�����м�����ݴ������
		double sqNR = sqrt(1 - pow(NR, 2));
		
		
		vec R;
		matrix3d T;

		//�˻�������������l�뷨��N�غ�
		if (N.cross_product(l).isZero())
		{
			R = N;
			vec tmp = vec(N);
			tmp.xyz[0] += 5;
			vec yvec = N.cross_product(tmp).normalized();
			vec xvec = yvec.cross_product(N).normalized();
			T = matrix3d(xvec, yvec, N);
		}
		else
		{
			R = ((N * (N.dot_product(l) * 2)) - l).normalized();
			//matrix3d T = matrix3d(N.cross_product(l).normalized(), N.cross_product(N.cross_product(l)).normalized(), N) * matrix3d(vec(1, 0, 0), vec(0, NR, -sqNR), vec(0, sqNR, NR));
			T = matrix3d(N.cross_product(l).normalized(), R.cross_product(N.cross_product(l).normalized()).normalized(), R);
		}
		
		//N��������任����RΪ�����xyz����ϵ
		vec na = (T.transpose() * N).normalized();

		double a1 = na.xyz[0];
		double a2 = na.xyz[1];
		double a3 = na.xyz[2];
		//double theta0 = atan(-a3 / (a1 * cos(phi) + a2 * sin(phi)));
		double theta0 = cal_theta0(a1, a2, a3, phi);

		//Rejection ����
		if (phi > 0 && phi < std::numbers::pi)
		{
			while (theta > theta0)
			{
				ksi1 = distribution(generator);
				theta = inverse_F_theta(x, y, n, ksi1, 1e-9);
			}
		}
		
		
		//�Է����rΪ�������theta,phi����ת������������ϵxyz���ꡣ
		vec v = T * vec(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

		//��ֹ�����ۼ����
		v = v.normalized();

		double M = cal_M(1.0 / (2 * std::numbers::pi * (x + y / (n + 1))), x, y, n, a1, a2, a3);
		
		//�۳��·���ĸ����ܶ�s
		double prob = (x + y * pow(cos(theta), n)) * sin(theta) / (2 * std::numbers::pi * (x + y / (n + 1.0))) * M;
		px *= prob;

		//����Radiance,��Russian Roulette����Ҫ��1/q�������ص�����
		Ii = shoot(x1, v, px, step) * (1.0 / q);
		
		//��·��ת�������õ���v�����Ϊ����x1��x�������������е�L����
		L = v;
		H = (L + V).normalized();
		if (N.dot_product(L) < 0 || H.dot_product(N) < 0)
		{
			double a = N.dot_product(L);
			double b = H.dot_product(N);
		}
	}
	
	//��ʼʹ��blin-phong����Radiance
	RadianceRGB kd = RadianceRGB(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]);
	RadianceRGB ks = RadianceRGB(mtl.specular[0], mtl.specular[1], mtl.specular[2]);
	double ns = mtl.shininess;
	RadianceRGB Ie = Ii * (kd * N.dot_product(L) + ks * pow(H.dot_product(N), ns));
	if (N.dot_product(L) < 0 || H.dot_product(N) < 0)
	{
		double a = N.dot_product(L);
		double b = H.dot_product(N);
	}
	/*if (Ie.RGB[0] < 0 || Ie.RGB[1] < 0 || Ie.RGB[2] < 0)
	{
		printf("147\n");
	}*/
	return Ie;
}
RadianceRGB buffer[720][1280];
int main()
{

	veach.read();
	//veach.loop();
	 //return 0;
	lights.read();
	lights.gather_light_triangles(veach.reader);
	
	//veach
	vec start = vec(28.2792, 5.2, 1.23612e-06);
	vec w = (vec(0.0, 2.8, 0.0) - start);

	//cornell
	//vec start = vec(278.0, 273.0, -800.0);
	//vec w = (vec(278.0, 273.0, -799.0) - start).normalized();

	//bathroom
	//vec start = vec(0.0072405338287353516, 0.9124049544334412, -0.2275838851928711);
	//vec w = (vec(-2.787562608718872, 0.9699121117591858, -2.6775901317596436) - start).normalized();

	veach.cal_scene_boundingbox(start);
	veach.meshing(100000);

	/*RadianceRGB sum(0, 0, 0);
	for (int i = 0; i < 100; i++)
	{
		std::cout << "Radiance: ";
		double p = 1;
		int step = 0;
		//sum = sum + shoot(start, w, p, step);
		//sum.print();
		RadianceRGB I = shoot(start, w, p, step) * (0.01 / p);
		I.print();
		sum = sum + I;
		std::cout << " p:" << p << " step:" << step << std::endl;
	}
	sum.print();
	*/

	// ��ʼ����ͼ����
	initgraph(1280, 720);

	// ��ȡָ����ʾ��������ָ��
	DWORD* pMem = GetImageBuffer();



	double pixellen = tan(20.1143 / 180) * w.norm2() / 640;
	vec eye = start;
	vec N = w.normalized();
	vec UP = vec(0, 1, 0);
	vec V = N.cross_product(UP).normalized();
	vec U = V.cross_product(N).normalized();
	matrix3d T = matrix3d(U, V, N);
//	vec dir2 = (T * (vec(0, 0, w.norm2()))).normalized();

	// ֱ�Ӷ���ʾ��������ֵ
	for (int i = 0; i < 720; i++)
	{
		printf("%d/720\n",i);
		for (int j = 0; j < 1280; j++)
		{
			printf("%d ", j);
			vec delta(-pixellen * (i - 359.5), pixellen * (j - 639.5), 0);
			vec dir = (T * (delta + vec(0, 0, w.norm2()))).normalized();

			RadianceRGB sum(0, 0, 0);
			for (int k = 0; k < 10; k++)
			{
				double p = 1;
				int step = 0;
				RadianceRGB I = shoot(eye, dir, p, step) * (0.1 / p);
				sum = sum + I;
			}
			//std::cout << "ra: ";
			//sum.print();
			//std::cout << std::endl;
			buffer[i][j] = sum;
			//#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
			pMem[i * 1280 + j] = BGR(((COLORREF)(((BYTE)(buffer[i][j].RGB[0]) | ((WORD)((BYTE)(buffer[i][j].RGB[1])) << 8)) | (((DWORD)(BYTE)(buffer[i][j].RGB[2])) << 16))));
		}
		FlushBatchDraw();
	}


	// ʹ��ʾ��������Ч
//	FlushBatchDraw();

	// ������Ƶ�ͼ��
	//saveimage(_T("C:\\Users\\luotong\\Desktop\\test.bmp"));
	saveimage(_T(".\\test.bmp"));
	system("pause");
	closegraph();
	return 0;
}