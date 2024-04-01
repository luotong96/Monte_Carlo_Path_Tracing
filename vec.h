#pragma once

class vec
{
public:
	double xyz[3];
	const double eps = 1e-8;

public:
	vec(double a = 0, double b = 0, double c = 0);
	vec(double a[]);

	void operator = (const vec& b);
	bool operator == (const vec& b)const;
	vec operator +(const vec& b)const;
	vec operator -(const vec& b)const;
	vec operator *(double c)const;
	double norm2()const;
	vec normalized()const;

	vec cross_product(const vec& b)const;
	double dot_product(const vec& b)const;
	void print()const;

	static double determinant(const vec& a, const vec& b, const vec& c);
	static vec get_zero_vector();
};