#pragma once
#include "vec.h"
#include <string>

class matrix3d
{
public:
	double A[3][3];
	
public:
	matrix3d();

	//��v1,v2,v3��Ϊ��������������
	matrix3d(vec v1, vec v2, vec v3);
	matrix3d(const matrix3d& b);

	//���������
	vec operator*(const vec& x)const;
	//����˾���
	matrix3d operator*(const matrix3d& b)const;

	matrix3d transpose()const;
};

