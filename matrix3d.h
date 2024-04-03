#pragma once
#include "vec.h"
#include <string>

class matrix3d
{
public:
	double A[3][3];
	
public:
	matrix3d();

	//用v1,v2,v3作为列向量构建矩阵
	matrix3d(vec v1, vec v2, vec v3);
	matrix3d(const matrix3d& b);

	//矩阵乘向量
	vec operator*(const vec& x)const;
	//矩阵乘矩阵
	matrix3d operator*(const matrix3d& b)const;

	matrix3d transpose()const;
};

