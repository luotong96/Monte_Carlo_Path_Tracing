#include "matrix3d.h"

matrix3d::matrix3d()
{
	memset(A, 0, sizeof(A));
}

matrix3d::matrix3d(vec v1, vec v2, vec v3)
{
	for (int i = 0; i < 3; i++)
	{
		A[i][0] = v1.xyz[i];
	}
	for (int i = 0; i < 3; i++)
	{
		A[i][1] = v2.xyz[i];
	}
	for (int i = 0; i < 3; i++)
	{
		A[i][2] = v3.xyz[i];
	}
}

matrix3d::matrix3d(const matrix3d& b)
{
	memcpy(A, b.A, sizeof(A));
}
//¾ØÕó³ËÏòÁ¿
vec matrix3d::operator*(const vec& x)const
{
	vec b(0, 0, 0);
	for (int i = 0; i < 3; i++)
	{
		for (int k = 0; k < 3; k++)
		{
			b.xyz[i] += A[i][k] * x.xyz[k];
		}
	}
	return b;
}
//¾ØÕó³Ë¾ØÕó
matrix3d matrix3d::operator*(const matrix3d& b)const
{
	const double(*B)[3] = b.A;
	matrix3d c;
	double(*C)[3] = c.A;
	for (int k = 0; k < 3; k++)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}
	return c;
}

matrix3d matrix3d::transpose()const
{
	matrix3d c;
	double(*C)[3] = c.A;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			C[i][j] = A[j][i];
		}
	}
	return c;
}
