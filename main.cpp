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
#include <graphics.h>		// 引用 EasyX 绘图库头文件
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
	//端点情况特殊处理
	if (ksi <= 0)
		return 0;
	if (ksi >= 1)
		return 0.5 * std::numbers::pi;
	//牛顿迭代，求F(theta)反函数。
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
	//另外一种a1 * cos(phi) + a2 * sin(phi) = 0的情况不存在，除非入射光与平面法向垂直。可画图分析
	if (fabs(a1) < eps && fabs(a2) < eps)
		return 0.5 * std::numbers::pi;
	return atan(-a3 / (a1 * cos(phi) + a2 * sin(phi)));
}

//integrand_func被积函数
inline double func(double phi, double x, double y, double n,double a1, double a2, double a3)
{
	//double theta0 = atan(-a3 / (a1 * cos(phi) + a2 * sin(phi)));
	double theta0 = cal_theta0(a1, a2, a3, phi);
	return x * cos(theta0) + y * pow(cos(theta0), n + 1) / (n + 1);
}

//计算Rejection采样中条件概率密度的乘积因子M。
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

//最新：shoot函数是失败的设计，无法正确运行，成功的设计在shade函数
//Path tracing，由摄像头发射光线。
//wi* x        .x2 wo*
//     .      .
//      .   .
//  w*   .x1
//从x点沿着w方向射出光线，w为单位向量。到达x点并沿w方向射出的累积概率密度是px。返回值是从光源射入x的radiance。
//递归向下求概率，回溯算radiance。xtri是x所属triangle
RadianceRGB shoot(vec x, vec w, double &px, int& step, triangle xtri)
{
	step++;
	//求交
	intersec_result rs = veach.closet_ray_intersect(x, w, xtri);
	if (rs.isIntersec == false)
		return RadianceRGB(0, 0, 0);

	vec x1 = linear_interpolation(veach.get_vertexes_of_facet(rs.s, rs.f), 1.0 - rs.beta - rs.gamma, rs.beta, rs.gamma);
	vec N = linear_interpolation(veach.get_normals_of_facet(rs.s, rs.f), 1.0 - rs.beta - rs.gamma, rs.beta, rs.gamma).normalized();
	//vec N = veach.get_unique_normal_of_facet(rs.s, rs.f); 
	if (N.dot_product(w * -1) < 0)
	{
		double tmp = N.dot_product(w * -1);
		printf("95\n");
		return RadianceRGB(0, 0, 0);
	}
	

	tinyobj::material_t mtl = veach.reader.GetMaterials().at(veach.reader.GetShapes().at(rs.s).mesh.material_ids[rs.f]);
	
	//用于回溯计算radiance的变量
	RadianceRGB Ii;
	vec V = w * (-1);
	vec L;
	vec H;

	//俄罗斯轮盘,以q的概率继续发射光线。q取kd（3项）和ks（3项）的6项平均值。
	double q = (mtl.diffuse[0] + mtl.diffuse[1] + mtl.diffuse[2] + mtl.specular[0] + mtl.specular[1] + mtl.specular[2]) / 6;
	q = fmin(1, q);

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();

	std::default_random_engine generator(seed1);
	std::uniform_real_distribution<double> distribution(0.0, 1.0);
	double ksi = distribution(generator);

	//停止继续发射光线。
	if (ksi >= q)
	{
		sampledLightPoint lightpoint = lights.select_a_point_from_lights(veach);
		px *= lightpoint.prob;

		vec wo = (lightpoint.coord - x1).normalized();
		L = wo;
		H = (L + V).normalized();

		//如果光源在当前facet的背面，则光路无效。
		if (L.dot_product(N) < 0)
		{
			Ii = RadianceRGB(0, 0, 0);
		}
		else {
			intersec_result rs1 = veach.closet_ray_intersect(x1, wo, triangle(rs.s,rs.f));
			//如果光源点与x1之间有遮挡，则光路无效，radiance贡献为0
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
		//从Phong模型的brdf中sample新的出射方向。
		//递归向下的时候，并不知道是什么颜色的光，因此直接使用kd各个维度的和。
		//这里使用-w作为入射光是因为目前还在从摄像头开始，递归向下的阶段。使用brdf来随机产生新的出射方向，目前的光路与回溯计算Radiance的光路是相反的。
		vec l = w * -1;
		double x = ((double)mtl.diffuse[0] + mtl.diffuse[1] + mtl.diffuse[2]) * N.dot_product(l);
		double y = (double)mtl.specular[0] + mtl.specular[1] + mtl.specular[2];
		double n = mtl.shininess;
		
		double ksi1 = distribution(generator);

		double theta = inverse_F_theta(x, y, n, ksi1, 1e-9);

		double ksi2 = distribution(generator);
		double phi = 2 * std::numbers::pi * ksi2;


		//specular反射中，N与反射光r的内积，等于N与入射光l的内积。
		double NR = N.dot_product(l);
		//sqNR仅为计算中间过程暂存变量。
		double sqNR = sqrt(1 - pow(NR, 2));
		
		
		vec R;
		matrix3d T;

		//退化情况：入射光线l与法向N重合
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
		
		//N向量坐标变换到以R为主轴的xyz坐标系
		vec na = (T.transpose() * N).normalized();

		double a1 = na.xyz[0];
		double a2 = na.xyz[1];
		double a3 = na.xyz[2];
		//double theta0 = atan(-a3 / (a1 * cos(phi) + a2 * sin(phi)));
		double theta0 = cal_theta0(a1, a2, a3, phi);

		//Rejection 采样
		if (phi > 0 && phi < std::numbers::pi)
		{
			while (theta > theta0)
			{
				ksi1 = distribution(generator);
				theta = inverse_F_theta(x, y, n, ksi1, 1e-9);
			}
		}
		
		
		//以反射光r为中心轴的theta,phi坐标转换到世界坐标系xyz坐标。
		vec v = T * vec(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

		//防止计算累计误差
		v = v.normalized();
		
		double M = cal_M(1.0 / (2 * std::numbers::pi * (x + y / (n + 1))), x, y, n, a1, a2, a3);
		
		//累乘新方向的概率密度s
		double prob = (x + y * pow(cos(theta), n)) * sin(theta) / (2 * std::numbers::pi * (x + y / (n + 1.0))) * M;
		px *= prob;

		//返回Radiance,有Russian Roulette，需要用1/q修正返回的能量
		Ii = shoot(x1, v, px, step,triangle(rs.s,rs.f)) * (1.0 / q);
		
		//光路反转，采样得到的v方向成为计算x1向x反射能量过程中的L方向。
		L = v;
		H = (L + V).normalized();
		if (N.dot_product(L) < 0 || H.dot_product(N) < 0)
		{
			double a = N.dot_product(L);
			double b = H.dot_product(N);
		}
	}
	
	//开始使用blin-phong计算Radiance
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

//point 相交点,wo视线方向
//自发光+直接光照光源采样+间接光照brdf采样
RadianceRGB shade(intersec_result point, vec wo)
{

	//当前点p,当前facet法向N
	vec p = linear_interpolation(veach.get_vertexes_of_facet(point.s, point.f), 1.0 - point.beta - point.gamma, point.beta, point.gamma);
	vec N = linear_interpolation(veach.get_normals_of_facet(point.s, point.f), 1.0 - point.beta - point.gamma, point.beta, point.gamma).normalized();
	
	//入射光线交在法向负半空间
	if (N.dot_product(wo) < 0)
	{
		return RadianceRGB(0, 0, 0);
	}

	//光源只考虑自发光
	RadianceRGB emit;
	if (lights.islight.find(triangle(point.s, point.f)) != lights.islight.end())
	{
		emit = lights.islight[triangle(point.s, point.f)];
		return emit;
	}


	//当前材质
	tinyobj::material_t mtl = veach.reader.GetMaterials().at(veach.reader.GetShapes().at(point.s).mesh.material_ids[point.f]); 


	// L_dir 直接光照的radiance
	RadianceRGB L_dir;
	sampledLightPoint lightpoint = lights.select_a_point_from_lights(veach);
	//sampledLightPoint lightpoint = lights.select_a_point_from_lights_spherical_triangle(p, N, veach);

	vec x1 = lightpoint.coord;
	//光源法向量
	vec n1 = veach.get_unique_normal_of_facet(lightpoint.s, lightpoint.f);
	//指向光源的入射方向
	vec wl = (x1 - p).normalized();

	//光线同时在当前facet和光源facet的正半平面
	if (wl.dot_product(N) > 0 && (wl * -1).dot_product(n1) > 0)
	{
		intersec_result rstmp = veach.closet_ray_intersect(p, wl, triangle(point.s, point.f));
		//当前点到光源中间无遮挡
		if (rstmp.isIntersec && rstmp.s == lightpoint.s && rstmp.f == lightpoint.f)
		{
			BRDF brdf = BRDF::get_brdf_phong(N, wl, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
			L_dir = lightpoint.I * brdf * (wl.dot_product(N) * (wl * -1).dot_product(n1) / (x1-p).dot_product(x1-p) / lightpoint.prob);
		}
	}

	//间接光照，初始为0
	RadianceRGB L_indir;
	//俄罗斯轮盘继续的概率
	double P_RR = 0.6;

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed1);
	std::uniform_real_distribution<double> distribution(0.0, 1.0);
	double ksi = distribution(generator);

	if (ksi > P_RR)
		return L_dir + L_indir;

	sampledRay wi = BRDF::sample_from_phong(N, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
	if (wi.dir.dot_product(N) < 0)
		return L_dir + L_indir;

	intersec_result rsq = veach.closet_ray_intersect(p, wi.dir, triangle(point.s, point.f));

	//相交于非光源。
	if (rsq.isIntersec && lights.islight.find(triangle(rsq.s, rsq.f)) == lights.islight.end())
	{
		BRDF brdf = BRDF::get_brdf_phong(N, wi.dir, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
		L_indir = shade(rsq, wi.dir * -1) * brdf * (wi.dir.dot_product(N) / wi.pdf / P_RR);
	}
	return L_dir + L_indir;
}

//point 相交点,wo视线方向
//自发光+brdf光线采样
RadianceRGB shade_with_brdf(intersec_result point, vec wo)
{
	//当前点p,当前facet法向N
	vec p = linear_interpolation(veach.get_vertexes_of_facet(point.s, point.f), 1.0 - point.beta - point.gamma, point.beta, point.gamma);
	vec N = linear_interpolation(veach.get_normals_of_facet(point.s, point.f), 1.0 - point.beta - point.gamma, point.beta, point.gamma).normalized();

	//入射光线交在法向负半空间
	if (N.dot_product(wo) < 0)
	{
		return RadianceRGB(0, 0, 0);
	}

	//光源只考虑自发光
	RadianceRGB emit;
	if (lights.islight.find(triangle(point.s, point.f)) != lights.islight.end())
	{
		emit = lights.islight[triangle(point.s, point.f)];
		return emit;
	}


	//当前材质
	tinyobj::material_t mtl = veach.reader.GetMaterials().at(veach.reader.GetShapes().at(point.s).mesh.material_ids[point.f]);

	//光照，初始为0
	RadianceRGB L;
	//俄罗斯轮盘继续的概率
	double P_RR = 0.8;

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed1);
	std::uniform_real_distribution<double> distribution(0.0, 1.0);
	double ksi = distribution(generator);

	if (ksi > P_RR)
		return  L;

	sampledRay wi = BRDF::sample_from_phong(N, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
	if (wi.dir.dot_product(N) < 0)
		return L;

	intersec_result rsq = veach.closet_ray_intersect(p, wi.dir, triangle(point.s, point.f));

	//与场景相交。
	if (rsq.isIntersec)
	{
		BRDF brdf = BRDF::get_brdf_phong(N, wi.dir, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
		L = shade_with_brdf(rsq, wi.dir * -1) * brdf * (wi.dir.dot_product(N) / wi.pdf / P_RR);
	}
	return L;

}

//Multiple Importance Sampling
RadianceRGB shade_with_mis(intersec_result point, vec wo)
{

	//当前点p,当前facet法向N
	vec p = linear_interpolation(veach.get_vertexes_of_facet(point.s, point.f), 1.0 - point.beta - point.gamma, point.beta, point.gamma);
	vec N = linear_interpolation(veach.get_normals_of_facet(point.s, point.f), 1.0 - point.beta - point.gamma, point.beta, point.gamma).normalized();

	//反射光线交在法向负半空间
	if (N.dot_product(wo) < 0)
	{
		return RadianceRGB(0, 0, 0);
	}

	//光源只考虑自发光
	RadianceRGB emit;
	if (lights.islight.find(triangle(point.s, point.f)) != lights.islight.end())
	{
		emit = lights.islight[triangle(point.s, point.f)];
		return emit;
	}


	//当前材质
	tinyobj::material_t mtl = veach.reader.GetMaterials().at(veach.reader.GetShapes().at(point.s).mesh.material_ids[point.f]);


	//俄罗斯轮盘继续的概率
	double P_RR = 0.6;

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed1);
	std::uniform_real_distribution<double> distribution(0.0, 1.0);
	double ksi = distribution(generator);

	if (ksi > P_RR)
		return RadianceRGB();


	// 光源采样的radiance Contribution
	RadianceRGB L_light;

	lights.prepared_for_lights_spherical_triangle_sampling(p, N, veach);
	sampledLightPoint lightpoint = lights.lights_spherical_triangle_sampling(p, N, veach);

	vec x1 = lightpoint.coord;
	//光源法向量
	vec n1 = veach.get_unique_normal_of_facet(lightpoint.s, lightpoint.f);
	//指向光源的入射方向
	vec wl = (x1 - p).normalized();

	//光线在N的正半平面
	if (wl.dot_product(N) > 0)
	{
		intersec_result rstmp = veach.closet_ray_intersect(p, wl, triangle(point.s, point.f));
		//当前点到光源中间无遮挡
		if (rstmp.isIntersec)
		{
			BRDF brdf = BRDF::get_brdf_phong(N, wl, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
			
			//当前光线采样出的方向wl在brdf采样下的概率密度
			double phongPdf = BRDF::eval_sample_from_phong_pdf(N, wl, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
			//注意结尾的MIS
			L_light = shade_with_mis(rstmp, wl * -1) * brdf * (wl.dot_product(N) / (lightpoint.prob + phongPdf) / P_RR);
		}
	}

	//来自brdf采样的光线contribution
	RadianceRGB L_brdf;
	
	sampledRay wi = BRDF::sample_from_phong(N, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
	if (wi.dir.dot_product(N) < 0)
		return L_light + L_brdf;

	intersec_result rsq = veach.closet_ray_intersect(p, wi.dir, triangle(point.s, point.f));

	//相交
	if (rsq.isIntersec)
	{
		BRDF brdf = BRDF::get_brdf_phong(N, wi.dir, wo, vec(mtl.diffuse[0], mtl.diffuse[1], mtl.diffuse[2]), vec(mtl.specular[0], mtl.specular[1], mtl.specular[2]), mtl.shininess);
		
		//wi在spherical triangle光线采样下的概率密度
		double light_pdf;
		intersec_result rs = veach.closet_ray_intersect_light_triangle(p, wi.dir, triangle(point.s, point.f), lights.islight);
		//若与光源三角形不相交，则概率密度为0
		if (rs.isIntersec)
			light_pdf = lights.eval_spherical_triangle_sampling_pdf(triangle(rs.s, rs.f), veach);
		else
			light_pdf = 0;

		L_brdf = shade_with_mis(rsq, wi.dir * -1) * brdf * (wi.dir.dot_product(N) / (wi.pdf + light_pdf) / P_RR);
	}
	return L_light + L_brdf;
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
	start = start - w;
	w = w * 2;
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

	// 初始化绘图窗口
	initgraph(1280, 720);

	// 获取指向显示缓冲区的指针
	DWORD* pMem = GetImageBuffer();

	

	//double pixellen = tan(20.1143 / 180) * w.norm2() / 640;
	double pixellen = tan(20.1143 / 360) * w.norm2() / 360;
	vec eye = start;
	vec N = w.normalized();
	vec UP = vec(0, 1, 0);
	vec V = N.cross_product(UP).normalized();
	vec U = V.cross_product(N).normalized();
	matrix3d T = matrix3d(U, V, N);
//	vec dir2 = (T * (vec(0, 0, w.norm2()))).normalized();

	// 直接对显示缓冲区赋值
	for (int i = 0; i < 720; i++)
	{
		printf("%d/720\n",i);
		for (int j = 0; j < 1280; j++)
		{
			//printf("%d ", j);
			vec delta(-pixellen * (i - 359.5), pixellen * (j - 639.5), 0);
			vec dir = (T * (delta + vec(0, 0, w.norm2()))).normalized();

			RadianceRGB sum(0, 0, 0);
			for (int k = 0; k < 10; k++)
			{
				double p = 1;
				int step = 0;
				//RadianceRGB I = shoot(eye, dir, p, step, triangle(-1, -1)) * (0.01 / p);
				intersec_result rs = veach.closet_ray_intersect(eye, dir, triangle(-1, -1));
				if (rs.isIntersec) 
				{
					RadianceRGB I = shade_with_brdf(rs, dir * -1) * 0.1;
					sum = sum + I;
				}
			}
			//std::cout << "ra: ";
			//sum.print();
			//std::cout << std::endl;
			buffer[i][j] = sum;
			std::array<int, 3> rgbi = sum.tone_mapping(380, 0.25);
			//#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
			pMem[i * 1280 + j] = BGR(((COLORREF)(((BYTE)(rgbi[0]) | ((WORD)((BYTE)(rgbi[1])) << 8)) | (((DWORD)(BYTE)(rgbi[2])) << 16))));
		}
		FlushBatchDraw();
	}


	// 使显示缓冲区生效
//	FlushBatchDraw();

	// 保存绘制的图像
	//saveimage(_T("C:\\Users\\luotong\\Desktop\\test.bmp"));
	saveimage(_T(".\\test.bmp"));
	system("pause");
	closegraph();
	return 0;
}