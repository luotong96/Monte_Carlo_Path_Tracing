#include "Mylight.h"
#include <iostream>
#include <random>
#include <chrono>

Mylight::Mylight(std::string inputfile)
{
	this->inputfile = inputfile;
}
void Mylight::read()
{
    std::cout << "正在读取光源xml文件数据..." << std::endl;
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(inputfile.c_str());
    if (!result)
    {
        std::cerr << "无法打开光源xml文件" << std::endl;
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
    std::cout << "光源xml文件数据读取完成" << std::endl;
}

void Mylight::gather_light_triangles(const tinyobj::ObjReader& reader)
{
    std::cout << "正在收集所有光源triangle..." << std::endl;

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(triangle)

        //循环每个面facet
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

            for (std::map<std::string, RadianceRGB>::iterator it = lightsRadiance.begin(); it!=lightsRadiance.end(); ++it)
            {

                //如果该面片的材质名称是光源的材质名称
                if (materials.at(shapes[s].mesh.material_ids[f]).name == it->first)
                {

                    //算三角面片的面积
                    vec abc[3];
                    for (size_t v = 0; v < 3; v++) {
                        // access to vertex
                        tinyobj::index_t idx = shapes[s].mesh.indices[3 * f + v];
                        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                        abc[v] = vec(vx, vy, vz);
                    }
                    //n为垂直于三角片单位法向量
                    vec n = (abc[1] - abc[0]).cross_product(abc[2] - abc[0]);
                    n = n * (1.0 / n.norm2());
                    //计算三角面片面积
                    double area = 0.5 * vec::determinant((abc[1] - abc[0]), (abc[2] - abc[0]), n);

                    lightTriangle ltri(s, f, area);

                    if (lightsTriangles.find(it->first) == lightsTriangles.end())
                    {
                        std::vector<lightTriangle> nv;
                        lightsTriangles[it->first] = nv;
                    }

                    //收集
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

    std::cout << "光源triangle收集完成" << std::endl;
}

sampledLightPoint Mylight::select_a_point_from_lights(const Myobj& myobj)
{
    //选择概率（密度）
    double proba = 1.0;

    unsigned seed1 = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed1);

    //先按照光源radiance强度随机

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

    //从同一辐射亮度的所有三角形中随机
    auto& lt = lightsTriangles.at(lightsName[rind]);
    std::vector<double> areas;
    for (auto& tri : lt)
    {
        areas.push_back(tri.area);
    }
    std::discrete_distribution<int> distribution2(areas.begin(), areas.end());
    int aind = distribution2(generator);

    proba *= distribution2.probabilities().at(aind);

    //在单个三角形内部均匀随机取点
    lightTriangle sel = lt.at(aind);

    //先在2d平面上beta>=0,gamma>=0，beta+gamma<=1的三角形中均匀采样。 
    std::uniform_real_distribution<double> distribution3(0.0, 1.0);

    double ksi1= distribution3(generator);
    double beta = 1 - sqrt(1 - ksi1);

    double ksi2 = distribution3(generator);
    double gamma = (1 - beta) * ksi2;

    double alpha = 1 - beta - gamma;

    //映射到空间中的三角形
    std::array<vec, 3> vs = myobj.get_vertexes_of_facet(sel.s, sel.f);

    vec point = (vs[0] * alpha) + (vs[1] * beta) + (vs[2] * gamma);
    RadianceRGB I = lightsRadiance.at(lightsName[rind]);
    proba *= 1.0 / sel.area;

    sampledLightPoint ans(sel.s, sel.f, point, I, proba);
    return ans;
}
