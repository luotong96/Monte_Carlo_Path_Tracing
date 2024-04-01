#include "vec.h"
#include <cmath>
#include <iostream>
//����ȫ�ֵ������������������任�ļ���

vec::vec(double a, double b, double c)
{
	xyz[0] = a; xyz[1] = b; xyz[2] = c;
}
vec::vec(double a[])
{
	xyz[0] = a[0]; xyz[1] = a[1]; xyz[2] = a[2];
}

//���ص��������
void vec::operator=(const vec& b)
{
	/*xyz[0] = b.xyz[0];
	xyz[1] = b.xyz[1];
	xyz[2] = b.xyz[2];*/
	memcpy(xyz,b.xyz,sizeof(xyz));
}

//������ȵ��ҽ������������
bool vec::operator == (const vec& b)const
{
	for (int i = 0; i < 3; i++)
	{
		if (fabs(xyz[i] - b.xyz[i]) > eps)
			return false;
	}
	return true;
}

vec vec::operator +(const vec& b)const
{
	return vec(xyz[0] + b.xyz[0], xyz[1] + b.xyz[1], xyz[2] + b.xyz[2]);
}

vec vec::operator -(const vec& b)const
{
	return vec(xyz[0] - b.xyz[0], xyz[1] - b.xyz[1], xyz[2] - b.xyz[2]);
}

//��������c
vec vec::operator *(double c)const
{
	return vec(xyz[0] * c, xyz[1] * c, xyz[2] * c);
}

//�������ȣ�2����
double vec::norm2()const
{
	return sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
}

//���������ֱ����3������ʽ�Ƴ������ɡ�
vec vec::cross_product(const vec & b)const
{
	return vec(xyz[1] * b.xyz[2] - xyz[2] * b.xyz[1], xyz[2] * b.xyz[0] - xyz[0] * b.xyz[2], xyz[0] * b.xyz[1] - xyz[1] * b.xyz[0]);
}

//�������
double vec::dot_product(const vec& b)const
{
	double ans = 0;
	for (int i = 0; i < 3; i++)
	{
		ans += xyz[i] * b.xyz[i];
	}
	return ans;
}

//��������ʽ���ȼ���������������������
double vec::determinant(const vec& a, const vec& b, const vec& c)
{
	return (a.cross_product(b)).dot_product(c);
}

vec vec::get_zero_vector()
{
	return vec(0, 0, 0);
}

void vec::print()const
{
	std::cout << xyz[0] << " " << xyz[1] << " " << xyz[2];
}

vec vec::normalized()const
{
	double len = norm2();
	return vec(xyz[0] / len, xyz[1] / len, xyz[2] / len);
}
