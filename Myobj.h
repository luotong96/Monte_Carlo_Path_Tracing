#pragma once

#include "tiny_obj_loader.h"
#include "vec.h"
#include <array>

struct intersec_result
{
    //是否成功与三角面片相交
    bool isIntersec;
    //交点所属三角面片所在shape的序号
    size_t s;
    //交点所属三角面片的序号
    size_t f;
    //三角形与光线求交过程中的beta,gamma,t参数结果 
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

    //ro光线起点，rd光线方向,一条光线与一个三角形相交
    intersec_result intersect_with_triangle(vec ro, vec rd, size_t s, size_t f)const;

    //ro光线起点，rd光线方向，找最近相交三角形
    intersec_result closet_ray_intersect(vec ro, vec rd)const;

    //返回指定facet的三个顶点向量。序号为f的facet 属于序号为s的shape
    std::array<vec,3> get_vertexes_of_facet(size_t s, size_t f) const;

    //返回指定facet的三个法向向量。序号为f的facet 属于序号为s的shape
    std::array<vec,3> get_normals_of_facet(size_t s, size_t f) const;


};

