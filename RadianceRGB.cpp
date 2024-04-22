#include "RadianceRGB.h"
#include <sstream>
#include <iostream>
#include <algorithm>

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

RadianceRGB RadianceRGB::operator *(const RadianceRGB& b)const
{
    return RadianceRGB(RGB[0] * b.RGB[0], RGB[1] * b.RGB[1], RGB[2] * b.RGB[2]);
}
RadianceRGB RadianceRGB::operator *(const BRDF& b)const
{
    return RadianceRGB(RGB[0] * b.RGB[0], RGB[1] * b.RGB[1], RGB[2] * b.RGB[2]);
}

std::array<int, 3> RadianceRGB::tone_mapping()
{
    std::array<int,3> rgb;
    for (int i = 0; i < 3; i++)
    {
        rgb[i] = std::max(std::min((int)floor(RGB[i] + 0.5), 255), 0);
    }
    return rgb;
}


double RadianceRGB::sum()
{
    return RGB[0] + RGB[1] + RGB[2];
}

void RadianceRGB::print()
{
    std::cout << RGB[0] << " " << RGB[1] << " " << RGB[2];
}


