#pragma once

#include "tiny_obj_loader.h"
#include "vec.h"
#include <array>

struct intersec_result
{
    //�Ƿ�ɹ���������Ƭ�ཻ
    bool isIntersec;
    //��������������Ƭ����shape�����
    size_t s;
    //��������������Ƭ�����
    size_t f;
    //������������󽻹����е�beta,gamma,t������� 
    double beta;
    double gamma;
    double t;
    intersec_result(bool isIntersec, size_t s, size_t f, double beta, double gamma, double t)
    {
        this->isIntersec = isIntersec;
        this->s = s;
        this->f = f;
        this->beta = beta;
        this->gamma = gamma;
        this->t = t;
    }
};
class Myobj
{
    const double eps = 1e-8;
public:
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    std::string inputfile ;

public:
    Myobj(std::string inputfile);
    void read();
    void loop();

    //ro������㣬rd���߷���,һ��������һ���������ཻ
    intersec_result intersect_with_triangle(vec ro, vec rd, size_t s, size_t f)const;

    //ro������㣬rd���߷���������ཻ������
    intersec_result closet_ray_intersect(vec ro, vec rd)const;

    //����ָ��facet�������������������Ϊf��facet �������Ϊs��shape
    std::array<vec,3> get_vertexes_of_facet(size_t s, size_t f) const;

    //����ָ��facet�������������������Ϊf��facet �������Ϊs��shape
    std::array<vec,3> get_normals_of_facet(size_t s, size_t f) const;


};

