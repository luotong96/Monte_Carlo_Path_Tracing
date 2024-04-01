#pragma once

#include "tiny_obj_loader.h"
#include "vec.h"
#include <array>

struct intersec_result
{
    //交点所属三角面片所在shape的序号
    size_t s;
    //交点所属三角面片的序号
    size_t f;
    //三角形与光线求交过程中的beta,gamma,t参数结果 
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

    //ro光线起点，rd光线方向
    intersec_result closet_ray_intersect(vec ro, vec rd);

    //返回指定facet的三个顶点向量。序号为f的facet 属于序号为s的shape
    std::array<vec,3> get_vertexes_of_facet(size_t s, size_t f) const;

    //返回指定facet的三个法向向量。序号为f的facet 属于序号为s的shape
    std::array<vec,3> get_normals_of_facet(size_t s, size_t f) const;


};

