#pragma once

#include "tiny_obj_loader.h"
#include "vec.h"
#include <array>

//光线求交结果
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
    intersec_result()
    {
        this->isIntersec = false;
        this->s = 0;
        this->f = 0;
        this->beta = 0;
        this->gamma = 0;
        this->t = 0;
    }

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

//用于meshing网格化
struct triangle
{
    //三角面片所在shape的序号
    size_t s;
    //三角面片的序号
    size_t f;

    triangle(size_t s, size_t f)
    {
        this->s = s;
        this->f = f;
    }
    bool operator< (const triangle& b)const
    {
        if (s != b.s)
            return s < b.s;
        return f < b.f;
    }
    bool operator==(const triangle& b)const
    {
        return (s == b.s) && (f == b.f);
    }
};
class Myobj
{
    const double eps = 1e-8;
public:
    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig reader_config;
    std::string inputfile ;

    //存储与每个网格overlapping的三角形
    std::vector<triangle> triangleOfGrid[100+3][100+3][100+3];

    //整个场景的boundingbox:xyz维度min and max
    double xyzmm[3][2];
    //单个网格正方形cell的宽度
    double gridCellWidth;
public:
    Myobj(std::string inputfile);
    void read();
    void loop();

    //计算整个场景的boundingbox，为网格化做准备
    void cal_scene_boundingbox(vec camera);
    //场景网格化，为光线求交做准备。
    void meshing(int c);

    //ro光线起点，rd光线单位方向,一条光线与一个三角形相交
    intersec_result intersect_with_triangle(vec ro, vec rd, size_t s, size_t f)const;

    //ro光线起点，rd光线方向，找最近相交三角形
    intersec_result closet_ray_intersect(vec ro, vec rd, triangle rotri)const;

    //返回指定facet的三个顶点向量。序号为f的facet 属于序号为s的shape
    std::array<vec,3> get_vertexes_of_facet(size_t s, size_t f) const;

    //返回指定facet的三个顶点法向向量。序号为f的facet 属于序号为s的shape
    std::array<vec,3> get_normals_of_facet(size_t s, size_t f) const;

    //计算并返回指定facet的唯一真实面法向量
    vec get_unique_normal_of_facet(size_t s, size_t f)const;
};

