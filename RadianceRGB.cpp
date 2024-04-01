#include "RadianceRGB.h"
#include <sstream>
#include <iostream>

RadianceRGB::RadianceRGB()
{
    memset(RGB, 0, sizeof(RGB));
}
RadianceRGB::RadianceRGB(double r,double g, double b)
{
    RGB[0] = r;
    RGB[1] = g;
    RGB[2] = b;
}

RadianceRGB::RadianceRGB(std::string radiance_str)
{
    std::stringstream ss(radiance_str);
    std::string item;

    for (int i = 0; i < 3; i++)
    {
        getline(ss, item, ',');
        this->RGB[i] = stod(item);
    }
}

RadianceRGB RadianceRGB::operator +(const RadianceRGB& b)
{
    return RadianceRGB(RGB[0] + b.RGB[0], RGB[1] + b.RGB[1], RGB[2] + b.RGB[2]);
}


RadianceRGB RadianceRGB::operator *(double c)const
{
    return RadianceRGB(RGB[0] * c, RGB[1] * c, RGB[2] * c);
}

double RadianceRGB::sum()
{
    return RGB[0] + RGB[1] + RGB[2];
}

void RadianceRGB::print()
{
    std::cout << RGB[0] << " " << RGB[1] << " " << RGB[2];
}