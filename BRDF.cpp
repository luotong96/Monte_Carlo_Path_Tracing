#include "BRDF.h"
#include <numbers>
#include <vector>
#include <random>
#include <chrono>
#include "matrix3d.h"

double BRDF::eps = 1e-8;

BRDF::BRDF(double a, double b, double c)
{
	RGB[0] = a; RGB[1] = b; RGB[2] = c;
}

//n为法向，wi为入射方向，wr为出射方向，wi、wr均在n所指向的正半空间,diffuse、specular、shiness为phong材质参数
//返回值为brdf函数三通道RGB的值
BRDF BRDF::get_brdf_phong(vec n, vec wi, vec wr, vec diffuse, vec specular, double shiness)
{
	//镜面反射主轴
	vec R = wi * -1 + n * (2 * (wi.dot_product(n)));
	vec ans = diffuse * (1.0 / std::numbers::pi);
	if(wr.dot_product(R) > 0)
		ans = ans + specular * ((shiness + 1) * pow(wr.dot_product(R), shiness) / (2 * std::numbers::pi));
	return BRDF(ans.xyz[0], ans.xyz[1], ans.xyz[2]);
}

//与get_brdf_phong参数含义一致，返回值为采样的入射方向wi，wi可能在n的负半空间
sampledRay BRDF::sample_from_phong(vec n, vec wr, vec diffuse, vec specular, double shiness)
{
	double d = diffuse.dot_product(vec(1, 1, 1)) / 3;
	double s = specular.dot_product(vec(1, 1, 1)) / 3;
	double pdf = 1;

	std::vector<double> weights;
	weights.push_back(d);
	weights.push_back(s);

	unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine generator(seed1);

	std::discrete_distribution<int> distribution(weights.begin(), weights.end());
	int ind = distribution(generator);
	pdf *= distribution.probabilities().at(ind);
	
	if (ind == 0)
	{
		//sample diffuse方向
		std::uniform_real_distribution<double> distribution2(0.0, 1.0);
		double ksi1 = distribution2(generator);
		double ksi2 = distribution2(generator);
		double theta = 0.5 * acos(fmax(-1,fmin(1,1 - 2 * ksi1)));
		double phi = 2 * std::numbers::pi * ksi2;

		pdf *= cos(theta) / std::numbers::pi;

		//以n为中心轴，变换到世界坐标系。
		
		vec nx;
		//n不可能与(1,0,0)、(0,1,0)同时共线
		if (fabs(n.dot_product(vec(1, 0, 0)) - 1) > eps)
		{
			nx = n.cross_product(vec(1, 0, 0)).normalized();
		}
		else
		{
			nx = n.cross_product(vec(0, 1, 0)).normalized();
		}
			
		vec ny = n.cross_product(nx).normalized();
		matrix3d T(nx, ny, n);
		vec dir = (T * vec(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta))).normalized();
		return sampledRay(dir, pdf);
	}
	else if (ind == 1)
	{
		//sample specular 方向
		std::uniform_real_distribution<double> distribution2(0.0, 1.0);
		double ksi1 = distribution2(generator);
		double ksi2 = distribution2(generator);
		double theta = acos(fmax(-1,fmin(1,pow(ksi1, 1 / (shiness + 1)))));
		double phi = 2 * std::numbers::pi * ksi2;
		pdf *= (shiness + 1) / (2 * std::numbers::pi) * pow(ksi1, shiness / (shiness + 1));

		//theta,phi以wr的完全镜面反射R方向为中心z轴，变换到世界坐标系

		vec R = ((wr * -1) + n * (2 * wr.dot_product(n))).normalized();
		//R不可能与(1,0,0)、(0,1,0)同时共线
		vec nx;
		if (fabs(R.dot_product(vec(1, 0, 0)) - 1) > eps)
		{
			nx = R.cross_product(vec(1, 0, 0)).normalized();
		}
		else
		{
			nx = R.cross_product(vec(0, 1, 0)).normalized();
		}
		vec ny = R.cross_product(nx).normalized();
		matrix3d T(nx, ny, R);
		vec dir = (T * vec(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta))).normalized();
		return sampledRay(dir, pdf);
	}

	return sampledRay(n * -1, 1);
}

//与get_brdf_phong参数含义一致，wi为采样的入射方向，wi可能在n的负半空间
double BRDF::eval_sample_from_phong_pdf(vec n, vec wi, vec wr, vec diffuse, vec specular, double shiness)
{
	double d = diffuse.dot_product(vec(1, 1, 1)) / 3;
	double s = specular.dot_product(vec(1, 1, 1)) / 3;
	double sum = d + s;
	//全概率公式
	double prob_d = d / sum;
	double prob_s = s / sum;
//先处理diffuse部分的条件概率
	double cosTheta = wi.dot_product(n);
	if (cosTheta < 0)
		prob_d *= 0;
	else
		prob_d *= cosTheta / std::numbers::pi;

//在处理specular部分的条件概率
	//R是wr关于法向n的完美镜面反射
	vec R = ((wr * -1) + n * (2 * wr.dot_product(n))).normalized();

	double cosThetaS = wi.dot_product(R);
	if (cosThetaS < 0)
		prob_s *= 0;
	else
		prob_s *= (shiness + 1) / (2 * std::numbers::pi) * pow(cosThetaS, shiness);
	
	return prob_d + prob_s;
}