
#include "Myobj.h"
#include <iostream>

Myobj::Myobj(std::string inputfile)
{
    this->inputfile = inputfile;
}

void Myobj::read()
{
    std::cout << "正在读取obj文件数据..." << std::endl;
    reader_config.mtl_search_path = ""; // Path to material files
    
    if (!reader.ParseFromFile(inputfile, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    std::cout << "总facet数量：" << reader.GetShapes()[0].mesh.num_face_vertices.size() << std::endl;
    std::cout << "obj文件数据读取完成" << std::endl;
}
void Myobj::loop()
{
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        std::cout << "总facet数量：" << shapes[s].mesh.num_face_vertices.size() << std::endl;
        //循环每个面facet
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }

                // Optional: vertex colors
                // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }
}


//一条光线与一个三角形相交
intersec_result Myobj::intersect_with_triangle(vec ro, vec rd, size_t s,size_t f)const
{
    std::array<vec, 3> abc = get_vertexes_of_facet(s, f);
    vec a = abc[0];
    vec b = abc[1];
    vec c = abc[2];

    double detA = vec::determinant(a - b, a - c, rd);

    //detA=0
    if (fabs(detA) < eps)
    {
        return intersec_result(false, s, f, 0, 0, 0);
    }

    double beta = vec::determinant(a - ro, a - c, rd) / detA;
    double gamma = vec::determinant(a - b, a - ro, rd) / detA;
    double t = vec::determinant(a - b, a - c, a - ro) / detA;

    //交点不在三角形内部
    if (beta < 0 || gamma < 0 || beta + gamma > 1 || t < 0||fabs(t)<eps)
    {
        return intersec_result(false, s, f, 0, 0, 0);
    }

    //成功相交
    return intersec_result(true, s, f, beta, gamma, t);
}

//ro光线起点，rd光线方向的单位向量
intersec_result Myobj::closet_ray_intersect(vec ro, vec rd)const
{
    intersec_result ans(false, 0, 0, 0, 0, DBL_MAX);
    auto& shapes = reader.GetShapes();
    for (size_t s = 0; s < shapes.size(); s++) {
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            intersec_result rs = intersect_with_triangle(ro, rd, s, f);
            if (rs.isIntersec)
            {
                if (rs.t < ans.t)
                {
                    ans = rs;
                }
            }
        }
    }
    return ans;
}

//返回指定facet的三个顶点向量。序号为f的facet 属于序号为s的shape
std::array<vec, 3> Myobj::get_vertexes_of_facet(size_t s, size_t f) const
{
    std::array<vec, 3> points;
    
    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    for (int i = 0; i < 3; i++)
    {
        tinyobj::index_t idx = shapes[s].mesh.indices[3 * f + i];

        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        points[i] = vec(vx, vy, vz);
    }

    return points;
}

//返回指定facet的三个顶点法向向量。序号为f的facet 属于序号为s的shape
std::array<vec, 3> Myobj::get_normals_of_facet(size_t s, size_t f) const
{

    std::array<vec, 3> normals;

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    for (int i = 0; i < 3; i++)
    {
        tinyobj::index_t idx = shapes[s].mesh.indices[3 * f + i];

        tinyobj::real_t vx = attrib.normals[3 * size_t(idx.normal_index) + 0];
        tinyobj::real_t vy = attrib.normals[3 * size_t(idx.normal_index) + 1];
        tinyobj::real_t vz = attrib.normals[3 * size_t(idx.normal_index) + 2];
        normals[i] = vec(vx, vy, vz);
    }

    return normals;
}

vec Myobj::get_unique_normal_of_facet(size_t s, size_t f)const
{
    std::array<vec, 3> abc = get_vertexes_of_facet(s, f);
    vec a = abc[0];
    vec b = abc[1];
    vec c = abc[2];
    
    std::array<vec, 3> nabc = get_normals_of_facet(s, f);
    vec na = nabc[0].normalized();
    vec nb = nabc[1].normalized();
    vec nc = nabc[2].normalized();

    vec n =(b - a).cross_product(c - a).normalized();
    vec nr = n * -1;
    double w = n.dot_product(na) + n.dot_product(nb) + n.dot_product(nc);
    double wr = nr.dot_product(na) + nr.dot_product(nb) + nr.dot_product(nc);
    return n;
    if (w > wr)
    {
        return n;
    }
    return nr;
}
