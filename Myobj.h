#pragma once

#include "tiny_obj_loader.h"
#include "vec.h"
#include <array>

struct intersec_result
{
    //��������������Ƭ����shape�����
    size_t s;
    //��������������Ƭ�����
    size_t f;
    //������������󽻹����е�beta,gamma,t������� 
    double bgt[3];
};
class Myobj
{
public:
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    std::string inputfile ;

public:
    Myobj(std::string inputfile);
    void read();
    void loop();

    //ro������㣬rd���߷���
    intersec_result closet_ray_intersect(vec ro, vec rd);

    //����ָ��facet�������������������Ϊf��facet �������Ϊs��shape
    std::array<vec,3> get_vertexes_of_facet(size_t s, size_t f) const;

    //����ָ��facet�������������������Ϊf��facet �������Ϊs��shape
    std::array<vec,3> get_normals_of_facet(size_t s, size_t f) const;


};

