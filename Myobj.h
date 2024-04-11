#pragma once

#include "tiny_obj_loader.h"
#include "vec.h"
#include <array>

//�����󽻽��
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

//����meshing����
struct triangle
{
    //������Ƭ����shape�����
    size_t s;
    //������Ƭ�����
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

    //�洢��ÿ������overlapping��������
    std::vector<triangle> triangleOfGrid[100+3][100+3][100+3];

    //����������boundingbox:xyzά��min and max
    double xyzmm[3][2];
    //��������������cell�Ŀ��
    double gridCellWidth;
public:
    Myobj(std::string inputfile);
    void read();
    void loop();

    //��������������boundingbox��Ϊ������׼��
    void cal_scene_boundingbox(vec camera);
    //�������񻯣�Ϊ��������׼����
    void meshing(int c);

    //ro������㣬rd���ߵ�λ����,һ��������һ���������ཻ
    intersec_result intersect_with_triangle(vec ro, vec rd, size_t s, size_t f)const;

    //ro������㣬rd���߷���������ཻ������
    intersec_result closet_ray_intersect(vec ro, vec rd, triangle rotri)const;

    //����ָ��facet�������������������Ϊf��facet �������Ϊs��shape
    std::array<vec,3> get_vertexes_of_facet(size_t s, size_t f) const;

    //����ָ��facet���������㷨�����������Ϊf��facet �������Ϊs��shape
    std::array<vec,3> get_normals_of_facet(size_t s, size_t f) const;

    //���㲢����ָ��facet��Ψһ��ʵ�淨����
    vec get_unique_normal_of_facet(size_t s, size_t f)const;
};

