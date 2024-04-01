
#include "Myobj.h"
#include <iostream>

Myobj::Myobj(std::string inputfile)
{
    this->inputfile = inputfile;
}

void Myobj::read()
{
    std::cout << "���ڶ�ȡobj�ļ�����..." << std::endl;
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
    std::cout << "obj�ļ����ݶ�ȡ���" << std::endl;
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
        std::cout << "��facet������" << shapes[s].mesh.num_face_vertices.size() << std::endl;
        //ѭ��ÿ����facet
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

//ro������㣬rd���߷���ĵ�λ����
intersec_result Myobj::closet_ray_intersect(vec ro, vec rd)
{

    return intersec_result();
}

//����ָ��facet�������������������Ϊf��facet �������Ϊs��shape
std::array<vec, 3> Myobj::get_vertexes_of_facet(size_t s, size_t f) const
{

    return std::array<vec, 3>{vec(1, 2, 3), vec(4, 5, 6), vec(7, 8, 9)};
}

//����ָ��facet�������������������Ϊf��facet �������Ϊs��shape
std::array<vec, 3> Myobj::get_normals_of_facet(size_t s, size_t f) const
{
    return std::array<vec, 3>();

}