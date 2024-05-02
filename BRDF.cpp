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

//nΪ����wiΪ���䷽��wrΪ���䷽��wi��wr����n��ָ�������ռ�,diffuse��specular��shinessΪphong���ʲ���
//����ֵΪbrdf������ͨ��RGB��ֵ
BRDF BRDF::get_brdf_phong(vec n, vec wi, vec wr, vec diffuse, vec specular, double shiness)
{
	//���淴������
	vec R = wi * -1 + n * (2 * (wi.dot_product(n)));
	vec ans = diffuse * (1.0 / std::numbers::pi);
	if(wr.dot_product(R) > 0)
		ans = ans + specular * ((shiness + 1) * pow(wr.dot_product(R), shiness) / (2 * std::numbers::pi));
	return BRDF(ans.xyz[0], ans.xyz[1], ans.xyz[2]);
}

//��get_brdf_phong��������һ�£�����ֵΪ���������䷽��wi��wi������n�ĸ���ռ�
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
		//sample diffuse����
		std::uniform_real_distribution<double> distribution2(0.0, 1.0);
		double ksi1 = distribution2(generator);
		double ksi2 = distribution2(generator);
		double theta = 0.5 * acos(fmax(-1,fmin(1,1 - 2 * ksi1)));
		double phi = 2 * std::numbers::pi * ksi2;

		pdf *= cos(theta) / std::numbers::pi;

		//��nΪ�����ᣬ�任����������ϵ��
		
		vec nx;
		//n��������(1,0,0)��(0,1,0)ͬʱ����
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
		//sample specular ����
		std::uniform_real_distribution<double> distribution2(0.0, 1.0);
		double ksi1 = distribution2(generator);
		double ksi2 = distribution2(generator);
		double theta = acos(fmax(-1,fmin(1,pow(ksi1, 1 / (shiness + 1)))));
		double phi = 2 * std::numbers::pi * ksi2;
		pdf *= (shiness + 1) / (2 * std::numbers::pi) * pow(ksi1, shiness / (shiness + 1));

		//theta,phi��wr����ȫ���淴��R����Ϊ����z�ᣬ�任����������ϵ

		vec R = ((wr * -1) + n * (2 * wr.dot_product(n))).normalized();
		//R��������(1,0,0)��(0,1,0)ͬʱ����
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

//��get_brdf_phong��������һ�£�wiΪ���������䷽��wi������n�ĸ���ռ�
double BRDF::eval_sample_from_phong_pdf(vec n, vec wi, vec wr, vec diffuse, vec specular, double shiness)
{
	double d = diffuse.dot_product(vec(1, 1, 1)) / 3;
	double s = specular.dot_product(vec(1, 1, 1)) / 3;
	double sum = d + s;
	//ȫ���ʹ�ʽ
	double prob_d = d / sum;
	double prob_s = s / sum;
//�ȴ���diffuse���ֵ���������
	double cosTheta = wi.dot_product(n);
	if (cosTheta < 0)
		prob_d *= 0;
	else
		prob_d *= cosTheta / std::numbers::pi;

//�ڴ���specular���ֵ���������
	//R��wr���ڷ���n���������淴��
	vec R = ((wr * -1) + n * (2 * wr.dot_product(n))).normalized();

	double cosThetaS = wi.dot_product(R);
	if (cosThetaS < 0)
		prob_s *= 0;
	else
		prob_s *= (shiness + 1) / (2 * std::numbers::pi) * pow(cosThetaS, shiness);
	
	return prob_d + prob_s;
}