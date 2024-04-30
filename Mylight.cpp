#include "Mylight.h"
#include <iostream>
#include <random>
#include <chrono>
#include <numbers>

Mylight::Mylight(std::string inputfile)
{
	this->inputfile = inputfile;
}
void Mylight::read()
{
    std::cout << "���ڶ�ȡ��Դxml�ļ�����..." << std::endl;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(inputfile.c_str());
    if (!result)
    {
        std::cerr << "�޷��򿪹�Դxml�ļ�" << std::endl;
        exit(1);
    }
    for (pugi::xml_node light : doc.children("light"))
    {
        std::string mtlname_str(light.attribute("mtlname").as_string());
        std::string radiance_str(light.attribute("radiance").as_string());
        RadianceRGB ra(radiance_str);
        lightsRadiance[mtlname_str] = ra;
        std::cout << mtlname_str <<"|"<< radiance_str << std::endl;
    }
    std::cout << "��Դxml�ļ����ݶ�ȡ���" << std::endl;
}

void Mylight::gather_light_triangles(const tinyobj::ObjReader& reader)
{
    std::cout << "�����ռ����й�Դtriangle..." << std::endl;

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(triangle)

        //ѭ��ÿ����facet
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

            for (std::map<std::string, RadianceRGB>::iterator it = lightsRadiance.begin(); it!=lightsRadiance.end(); ++it)
            {

                //�������Ƭ�Ĳ��������ǹ�Դ�Ĳ�������
                if (materials.at(shapes[s].mesh.material_ids[f]).name == it->first)
                {
                    islight[triangle(s, f)] = lightsRadiance[it->first];
                    
                    //��������Ƭ�����
                    vec abc[3];
                    for (size_t v = 0; v < 3; v++) {
                        // access to vertex
                        tinyobj::index_t idx = shapes[s].mesh.indices[3 * f + v];
                        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                        abc[v] = vec(vx, vy, vz);
                    }
                    //nΪ��ֱ������Ƭ��λ������
                    vec n = (abc[1] - abc[0]).cross_product(abc[2] - abc[0]);
                    n = n * (1.0 / n.norm2());
                    //����������Ƭ���
                    double area = 0.5 * vec::determinant((abc[1] - abc[0]), (abc[2] - abc[0]), n);

                    lightTriangle ltri(s, f, area);

                    if (lightsTriangles.find(it->first) == lightsTriangles.end())
                    {
                        std::vector<lightTriangle> nv;
                        lightsTriangles[it->first] = nv;
                    }

                    //�ռ�
                    lightsTriangles[it->first].push_back(ltri);

                    break;
                }
            }
        }
    }

    for (std::map<std::string, std::vector<lightTriangle> >::iterator it = lightsTriangles.begin(); it != lightsTriangles.end(); it++)
    {
        std::cout << it->first <<"|" << it->second.size() << "|" ;
        double sum = 0;
        for (auto& tri : it->second)
        {
            sum += tri.area;
        }
        std::cout << sum << std::endl;
    }

    std::cout << "��Դtriangle�ռ����" << std::endl;
}

sampledLightPoint Mylight::select_a_point_from_lights(Myobj& myobj)
{
    //ѡ����ʣ��ܶȣ�
    double proba = 1.0;

    unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed1);

    //�Ȱ��չ�Դradianceǿ�����

    std::vector<std::string> lightsName;
    std::vector<double> lightsRa;
    for (auto keyvalue : lightsRadiance)
    {
        lightsName.push_back(keyvalue.first);
        lightsRa.push_back(keyvalue.second.sum());
    }

    std::discrete_distribution<int> distribution1(lightsRa.begin(),lightsRa.end());
    int rind = distribution1(generator);

    proba *= distribution1.probabilities().at(rind);

    //��ͬһ�������ȵ����������������
    auto& lt = lightsTriangles.at(lightsName[rind]);
    std::vector<double> areas;
    for (auto& tri : lt)
    {
        areas.push_back(tri.area);
    }
    std::discrete_distribution<int> distribution2(areas.begin(), areas.end());
    int aind = distribution2(generator);

    proba *= distribution2.probabilities().at(aind);

    //�ڵ����������ڲ��������ȡ��
    lightTriangle sel = lt.at(aind);

    //����2dƽ����beta>=0,gamma>=0��beta+gamma<=1���������о��Ȳ����� 
    std::uniform_real_distribution<double> distribution3(0.0, 1.0);

    double ksi1= distribution3(generator);
    double beta = 1 - sqrt(1 - ksi1);

    double ksi2 = distribution3(generator);
    double gamma = (1 - beta) * ksi2;

    double alpha = 1 - beta - gamma;

    //ӳ�䵽�ռ��е�������
    std::array<vec, 3> vs = myobj.get_vertexes_of_facet(sel.s, sel.f);

    vec point = (vs[0] * alpha) + (vs[1] * beta) + (vs[2] * gamma);
    RadianceRGB I = lightsRadiance.at(lightsName[rind]);
    proba *= 1.0 / sel.area;

    sampledLightPoint ans(sel.s, sel.f, point, I, proba);
    return ans;
}

//x1��ǰ�㣬n�ǵ�ǰ������ƽ�淨����
sampledLightPoint Mylight::select_a_point_from_lights_spherical_triangle(vec x1, vec n, Myobj& myobj)
{
    std::vector<projectedSphericalTriangle> sphericalTriangles;
    //����ѡ��ĳһ�������ι�Դ��Ȩ�أ��������ڵ�ǰ���ͶӰ���*Radiance
    std::vector<double> weights;
    double weights_sum = 0;
    for (auto it = lightsTriangles.begin(); it != lightsTriangles.end(); ++it)
    {
        std::string mtlname = it->first;
        auto& triangles = it->second;
        for (auto& tri : triangles)
        {
            std::array<vec, 3> points = myobj.get_vertexes_of_facet(tri.s, tri.f);

            //��Դ�����εķ�����
            vec nl = myobj.get_unique_normal_of_facet(tri.s, tri.f);

            double tmp = nl.dot_product(x1 - points[0]);
            //����x1�ڹ�Դ�ı��棬�������ù�Դ�����ڻ�<=0
            if (tmp < 0 || fabs(tmp) < eps)
            {
                continue;
            }

            double tmps[3];
            for (int i = 0; i < 3; i++)
            {
                tmps[i] = n.dot_product(points[i] - x1);
            }
            
            //��Դ��ȫ��x1����ƽ��ı���
            if ((tmps[0] < 0 || fabs(tmps[0]) < eps) && (tmps[1] < 0 || fabs(tmps[1]) < eps) && (tmps[2] < 0 || fabs(tmps[2]) < eps))
            {
                continue;
            }
            

            vec A = (points[0] - x1).normalized();
            vec B = (points[1] - x1).normalized();
            vec C = (points[2] - x1).normalized();
            //������Ҫ�ϸ���һ��������Ƭ����Ͷ����˳��Ĺ�ϵ���Ӷ���A,B,C��Arvo�㷨�Ķ���˳����롣
            //�����ж��޷����������θպ���ƽ�洹ֱ�������
            double aaaaa = (C - A).cross_product(B - A).dot_product(n);
            if ((C - A).normalized().cross_product((B - A).normalized()).dot_product(n) < 0)
            {
                vec tmpv = B;
                B = C;
                C = tmpv;
            }

            //DefineTriangle(A,B,C)
            
            double a = acos(fmax(-1, fmin(1, B.dot_product(C))));
            double b = acos(fmax(-1, fmin(1, A.dot_product(C))));
            double c = acos(fmax(-1, fmin(1, A.dot_product(B))));
            //ĳ�����߳���̫С���������˻�������
            if (a < eps || b < eps || c < eps)
            {
                continue;
            }


            double alpha = acos(fmax(-1, fmin(1, -B.cross_product(A).normalized().dot_product(A.cross_product(C).normalized()))));
            double beta = acos(fmax(-1, fmin(1, -C.cross_product(B).normalized().dot_product(B.cross_product(A).normalized()))));
            double gamma = acos(fmax(-1, fmin(1, -A.cross_product(C).normalized().dot_product(C.cross_product(B).normalized()))));
            
            //���ĳ����̫С����ͶӰ�������˻���������
            if (alpha < eps || beta < eps || gamma < eps)
            {
                continue;
            }
           

            double sA = alpha + beta + gamma - std::numbers::pi;

            if (sA < 0)
            {
                continue;
            }

            RadianceRGB L = lightsRadiance.at(mtlname);
            double weight = sA * L.sum();
            
            if (weight < 0 )
            {
                continue;
            }
            if (isinf(weight) || isnan(weight))
            {
                continue;
            }

            sphericalTriangles.push_back(projectedSphericalTriangle(A, B, C, alpha, beta, gamma, a, b, c, sA, tri.s, tri.f, L));
            weights.push_back(weight);
            weights_sum += weight;
        }
    }
    //printf("###%d %f###\n", weights.size(), weights_sum);

    //��Ȩֵ̫С���˳���
    if (sphericalTriangles.size() == 0|| weights.size() == 0 || fabs(weights_sum) < eps )
    {
        return sampledLightPoint(0, 0, n * -1 + x1, RadianceRGB(), 1);
    }

    unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed1);

    std::discrete_distribution<int> distribution1(weights.begin(), weights.end());

    //����ѡ��sphericalTriangles���±�Ϊrind�Ĺ�Դ��
    int rind = distribution1(generator);
    double prob = distribution1.probabilities().at(rind);
    auto& st = sphericalTriangles.at(rind);
    //st.a = acos(fmax(-1, fmin(1,st.B.dot_product(st.C))));
    //st.b = acos(fmax(-1, fmin(1,st.A.dot_product(st.C))));
    //st.c = acos(fmax(-1, fmin(1,st.A.dot_product(st.B))));



    std::uniform_real_distribution<double> distribution2(0.0, 1.0);

    double ksi1 = distribution2(generator);
    double ksi2 = distribution2(generator);

    //SampleTriangle(ksi1,ksi2)
    double sA1 = ksi1 * st.sA;
    double s = sin(sA1 - st.alpha);
    double t = cos(sA1 - st.alpha);
    double u = t - cos(st.alpha);
    double v = s + sin(st.alpha) * cos(st.c);
    double q = ((v * t - u * s) * cos(st.alpha) - v) / ((v * s + u * t) * sin(st.alpha));
    vec C1 = (st.A * q + (st.C - st.A * (st.C.dot_product(st.A))).normalized() * sqrt(1 - q * q)).normalized();
    double z = 1 - ksi2 * (1 - C1.dot_product(st.B));
    vec P = (st.B * z + (C1 - st.B * (C1.dot_product(st.B))).normalized() * sqrt(1 - z * z)).normalized();
    
   /* double yyy = P.dot_product(n);
    if ( yyy < 0)
    {
        printf("no\n"); 
    }
    double xxx = (P * -1).dot_product(myobj.get_unique_normal_of_facet(st.s, st.f));
    if (xxx < 0)
    {
        printf("nono\n");
    }*/ 
    //return sampledLightPoint(st.s, st.f, P + x1, st.L, prob / st.sA);

    intersec_result rs = myobj.intersect_with_triangle(x1, P, st.s, st.f);
    if (!rs.isIntersec)
    {
        int a = 1;
    }

    return sampledLightPoint(st.s, st.f, x1 + P * rs.t, st.L, st.L.sum() / weights_sum);
}


//x1��ǰ�㣬n�ǵ�ǰ������ƽ�淨����
void Mylight::prepared_for_lights_spherical_triangle_sampling(vec x1, vec n, Myobj& myobj)
{
    sphericalTriangles.clear();
    weights.clear();
    weights_sum = 0;

    for (auto it = lightsTriangles.begin(); it != lightsTriangles.end(); ++it)
    {
        std::string mtlname = it->first;
        auto& triangles = it->second;
        for (auto& tri : triangles)
        {
            std::array<vec, 3> points = myobj.get_vertexes_of_facet(tri.s, tri.f);

            //��Դ�����εķ�����
            vec nl = myobj.get_unique_normal_of_facet(tri.s, tri.f);

            double tmp = nl.dot_product(x1 - points[0]);
            //����x1�ڹ�Դ�ı��棬�������ù�Դ�����ڻ�<=0
            if (tmp < 0 || fabs(tmp) < eps)
            {
                continue;
            }

            double tmps[3];
            for (int i = 0; i < 3; i++)
            {
                tmps[i] = n.dot_product(points[i] - x1);
            }

            //��Դ��ȫ��x1����ƽ��ı���
            if ((tmps[0] < 0 || fabs(tmps[0]) < eps) && (tmps[1] < 0 || fabs(tmps[1]) < eps) && (tmps[2] < 0 || fabs(tmps[2]) < eps))
            {
                continue;
            }


            vec A = (points[0] - x1).normalized();
            vec B = (points[1] - x1).normalized();
            vec C = (points[2] - x1).normalized();
            //������Ҫ�ϸ���һ��������Ƭ����Ͷ����˳��Ĺ�ϵ���Ӷ���A,B,C��Arvo�㷨�Ķ���˳����롣
            //�����ж��޷����������θպ���ƽ�洹ֱ�������
            double aaaaa = (C - A).cross_product(B - A).dot_product(n);
            if ((C - A).normalized().cross_product((B - A).normalized()).dot_product(n) < 0)
            {
                vec tmpv = B;
                B = C;
                C = tmpv;
            }

            //DefineTriangle(A,B,C)

            double a = acos(fmax(-1, fmin(1, B.dot_product(C))));
            double b = acos(fmax(-1, fmin(1, A.dot_product(C))));
            double c = acos(fmax(-1, fmin(1, A.dot_product(B))));
            //ĳ�����߳���̫С���������˻�������
            if (a < eps || b < eps || c < eps)
            {
                continue;
            }


            double alpha = acos(fmax(-1, fmin(1, -B.cross_product(A).normalized().dot_product(A.cross_product(C).normalized()))));
            double beta = acos(fmax(-1, fmin(1, -C.cross_product(B).normalized().dot_product(B.cross_product(A).normalized()))));
            double gamma = acos(fmax(-1, fmin(1, -A.cross_product(C).normalized().dot_product(C.cross_product(B).normalized()))));

            //���ĳ����̫С����ͶӰ�������˻���������
            if (alpha < eps || beta < eps || gamma < eps)
            {
                continue;
            }


            double sA = alpha + beta + gamma - std::numbers::pi;

            if (sA < 0)
            {
                continue;
            }

            RadianceRGB L = lightsRadiance.at(mtlname);
            double weight = sA * L.sum();

            if (weight < 0)
            {
                continue;
            }
            if (isinf(weight) || isnan(weight))
            {
                continue;
            }

            indiceMap[triangle(tri.s, tri.f)] = sphericalTriangles.size();
            sphericalTriangles.push_back(projectedSphericalTriangle(A, B, C, alpha, beta, gamma, a, b, c, sA, tri.s, tri.f, L));
            weights.push_back(weight);
            weights_sum += weight;
            
        }
    }
}

sampledLightPoint Mylight::lights_spherical_triangle_sampling(vec x1, vec n, Myobj& myobj)
{
    //��Ȩֵ̫С���˳���
    if (sphericalTriangles.size() == 0 || weights.size() == 0 || fabs(weights_sum) < eps)
    {
        return sampledLightPoint(0, 0, n * -1 + x1, RadianceRGB(), 1);
    }

    unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed1);

    std::discrete_distribution<int> distribution1(weights.begin(), weights.end());

    //����ѡ��sphericalTriangles���±�Ϊrind�Ĺ�Դ��
    int rind = distribution1(generator);
    double prob = distribution1.probabilities().at(rind);
    auto& st = sphericalTriangles.at(rind);
    //st.a = acos(fmax(-1, fmin(1,st.B.dot_product(st.C))));
    //st.b = acos(fmax(-1, fmin(1,st.A.dot_product(st.C))));
    //st.c = acos(fmax(-1, fmin(1,st.A.dot_product(st.B))));



    std::uniform_real_distribution<double> distribution2(0.0, 1.0);

    double ksi1 = distribution2(generator);
    double ksi2 = distribution2(generator);

    //SampleTriangle(ksi1,ksi2)
    double sA1 = ksi1 * st.sA;
    double s = sin(sA1 - st.alpha);
    double t = cos(sA1 - st.alpha);
    double u = t - cos(st.alpha);
    double v = s + sin(st.alpha) * cos(st.c);
    double q = ((v * t - u * s) * cos(st.alpha) - v) / ((v * s + u * t) * sin(st.alpha));
    vec C1 = (st.A * q + (st.C - st.A * (st.C.dot_product(st.A))).normalized() * sqrt(1 - q * q)).normalized();
    double z = 1 - ksi2 * (1 - C1.dot_product(st.B));
    vec P = (st.B * z + (C1 - st.B * (C1.dot_product(st.B))).normalized() * sqrt(1 - z * z)).normalized();

    /* double yyy = P.dot_product(n);
     if ( yyy < 0)
     {
         printf("no\n");
     }
     double xxx = (P * -1).dot_product(myobj.get_unique_normal_of_facet(st.s, st.f));
     if (xxx < 0)
     {
         printf("nono\n");
     }*/
     //return sampledLightPoint(st.s, st.f, P + x1, st.L, prob / st.sA);

    intersec_result rs = myobj.intersect_with_triangle(x1, P, st.s, st.f);
    if (!rs.isIntersec)
    {
        int a = 1;
    }

    return sampledLightPoint(st.s, st.f, x1 + P * rs.t, st.L, st.L.sum() / weights_sum);
}

double Mylight::eval_spherical_triangle_sampling_pdf(triangle tri, Myobj& myobj)
{
    if (indiceMap.find(tri) == indiceMap.end())
        return 0;
    return sphericalTriangles.at(indiceMap.at(tri)).L.sum() / weights_sum;
}
