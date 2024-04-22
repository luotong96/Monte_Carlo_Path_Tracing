#include "BRDF.h"
#include <numbers>
#include <vector>
#include <random>
#include <chrono>
#include "matrix3d.h"
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
		double theta = 0.5 * acos(1 - 2 * ksi1);
		double phi = 2 * std::numbers::pi * ksi2;

		pdf *= cos(theta) / std::numbers::pi;
		return sampledRay(vec(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)), pdf);
	}
	else if (ind == 1)
	{
		//sample specular ����
		std::uniform_real_distribution<double> distribution2(0.0, 1.0);
		double ksi1 = distribution2(generator);
		double ksi2 = distribution2(generator);
		double theta = acos(pow(ksi1, 1 / (shiness + 1)));
		double phi = 2 * std::numbers::pi * ksi2;
		pdf *= (shiness + 1) / (2 * std::numbers::pi) * pow(ksi1, shiness / (shiness + 1));

		//theta,phi��wrΪ����z�ᣬ�任����������ϵ
		vec nx = wr.cross_product((wr + vec(0, 0, 10)).normalized()).normalized();
		vec ny = wr.cross_product(nx).normalized();
		matrix3d T(nx, ny, wr);
		vec dir = T * vec(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta)).normalized();
		
		return sampledRay(dir, pdf);
	}

	return sampledRay(n * -1, 1);
}
