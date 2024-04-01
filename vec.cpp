#include "vec.h"
#include <cmath>
#include <iostream>
//定义全局的向量，方便后续坐标变换的计算

vec::vec(double a, double b, double c)
{
	xyz[0] = a; xyz[1] = b; xyz[2] = c;
}
vec::vec(double a[])
{
	xyz[0] = a[0]; xyz[1] = a[1]; xyz[2] = a[2];
}

//重载等于运算符
void vec::operator=(const vec& b)
{
	/*xyz[0] = b.xyz[0];
	xyz[1] = b.xyz[1];
	xyz[2] = b.xyz[2];*/
	memcpy(xyz,b.xyz,sizeof(xyz));
}

//向量相等当且仅当各坐标相等
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

//向量数乘c
vec vec::operator *(double c)const
{
	return vec(xyz[0] * c, xyz[1] * c, xyz[2] * c);
}

//向量长度，2范数
double vec::norm2()const
{
	return sqrt(xyz[0] * xyz[0] + xyz[1] * xyz[1] + xyz[2] * xyz[2]);
}

//向量叉积，直接用3阶行列式推出来即可。
vec vec::cross_product(const vec & b)const
{
	return vec(xyz[1] * b.xyz[2] - xyz[2] * b.xyz[1], xyz[2] * b.xyz[0] - xyz[0] * b.xyz[2], xyz[0] * b.xyz[1] - xyz[1] * b.xyz[0]);
}

//向量点积
double vec::dot_product(const vec& b)const
{
	double ans = 0;
	for (int i = 0; i < 3; i++)
	{
		ans += xyz[i] * b.xyz[i];
	}
	return ans;
}

//三阶行列式，等价于先做叉积，再做点积。
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
