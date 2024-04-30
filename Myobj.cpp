
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

void Myobj::cal_scene_boundingbox(vec camera)
{
    //先求全局的boundingbox
    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    //最小值和最大值的初值为camera所在位置
    for (int i = 0; i < 3; i++)
    {
        xyzmm[i][0] = xyzmm[i][1] = camera.xyz[i];
    }
    for (size_t s = 0; s < shapes.size(); s++) {
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[3 * f + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                for (int i = 0; i < 3; i++)
                {
                    xyzmm[i][0] = fmin(xyzmm[i][0], attrib.vertices[3 * size_t(idx.vertex_index) + i]);
                    xyzmm[i][1] = fmax(xyzmm[i][1], attrib.vertices[3 * size_t(idx.vertex_index) + i]);
                }
            }
        }
    }
}

//n0是网格的初始个数，生成的网格数量可能会因为精度问题稍微多一些。
void Myobj::meshing(int n0)
{
    double len[3];
    for (int i = 0; i < 3; i++)
    {
        len[i] = xyzmm[i][1] - xyzmm[i][0];
    }
    //一个网格的长度
    //整体场景矩形网格而非正方形网格方案：double d = sqrt(len[0] * len[1] * len[2] / c);
    double d = fmax(fmax(len[0], len[1]), len[2]) / pow(n0, 1.0 / 3);
    gridCellWidth = d;

    //开始网格化
    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    for (size_t s = 0; s < shapes.size(); s++) {
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            //求当前面片的boundingbox
            double xyz[3][2];
            for (int k = 0; k < 3; k++)
            {
                xyz[k][0] = DBL_MAX;
                xyz[k][1] = -DBL_MAX;
            }
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[3 * f + v];
                for (int i = 0; i < 3; i++)
                {
                    xyz[i][0] = fmin(xyz[i][0], attrib.vertices[3 * size_t(idx.vertex_index) + i]);
                    xyz[i][1] = fmax(xyz[i][1], attrib.vertices[3 * size_t(idx.vertex_index) + i]);
                }
            }
            int xyzi[3][2];
            for (int k = 0; k < 3; k++)
            {
                xyzi[k][0] = (int)floor((xyz[k][0] - xyzmm[k][0]) / d);
                xyzi[k][1] = (int)floor((xyz[k][1] - xyzmm[k][0]) / d);
            }
            for (int i = xyzi[0][0]; i <= xyzi[0][1]; i++)
            {
                for (int j = xyzi[1][0]; j <= xyzi[1][1]; j++)
                {
                    for (int k = xyzi[2][0]; k <= xyzi[2][1]; k++)
                    {
                        triangleOfGrid[i][j][k].push_back(triangle(s, f));
                    }
                }
            }
        }
    }
}

//一条光线与一个三角形相交
intersec_result Myobj::intersect_with_triangle(vec ro, vec rd, size_t s,size_t f)
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
    if (beta < 0 || gamma < 0 || beta + gamma > 1 || t < 0|| fabs(t) < eps)
    {
        return intersec_result(false, s, f, 0, 0, 0);
    }

    //成功相交
    return intersec_result(true, s, f, beta, gamma, t);
}

/*暴力枚举版本
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
*/
//ro光线起点，rd光线方向的单位向量
/*intersec_result Myobj::closet_ray_intersect(vec ro, vec rd)const
{
    std::map<triangle, intersec_result> intersecMap;

    intersec_result ans(false, 0, 0, 0, 0, DBL_MAX);
    //防止自相交
    //const double delta = 1e-5;
    //ro = ro + rd * delta;
    //变换到网格坐标上，该网格的整数点坐标在像素块中心。
    vec xyz0 = (ro - vec(xyzmm[0][0], xyzmm[1][0], xyzmm[2][0])) * (1.0 / gridCellWidth) - vec(0.5, 0.5, 0.5);
    vec abc = (rd * (1.0 / gridCellWidth)).normalized();
    
    //网格整点坐标
    int xyz[3];
    for (int i = 0; i < 3; i++)
    {
        xyz[i] = (int)floor(xyz0.xyz[i] + 0.5);
    }

    //sign(a,b,c)
    int sign[3];
    for (int i = 0; i < 3; i++)
    {
        sign[i] = abc.xyz[i] < 0 ? -1 : 1;
        if (fabs(abc.xyz[i]) < eps)
        {
            sign[i] = 0;
        }
    }
    vec e = xyz0 + abc - vec(xyz) - vec(sign) * 0.5;
    
    //整个场景的boundingbox边长
    double len[3];
    for (int i = 0; i < 3; i++)
    {
        len[i] = xyzmm[i][1] - xyzmm[i][0];
    }

    bool reachbound = false;
    int cnta = (int)floor(len[0] / gridCellWidth);
    int cntb = (int)floor (len[1] / gridCellWidth);
    int cntc = (int)floor(len[2] / gridCellWidth);

    while (1)
    {
        for (int i = 0; i < 3; i++)
        {
            //越过网格边界
            if (xyz[i] < 0 || xyz[i] > (int)floor(len[i] / gridCellWidth) + 2)
            {
                reachbound = true;
                break;
            }
        }
        if (reachbound)
            break;

        for (auto &tri : triangleOfGrid[xyz[0]][xyz[1]][xyz[2]])
        {
            if (intersecMap.find(tri) == intersecMap.end())
            {
                intersecMap[tri] = intersect_with_triangle(ro, rd, tri.s, tri.f);
            }
            intersec_result rs = intersecMap[tri];
            if (rs.isIntersec)
            {
                bool outOfBox = false;
                //交点所在格子编号
                vec crosspoint = (ro + (rd * rs.t) - vec(xyzmm[0][0], xyzmm[1][0], xyzmm[2][0]))* (1.0 / gridCellWidth);
                int rsxyz[3];
                for (int i = 0; i < 3; i++)
                {
                    rsxyz[i] = (int)floor(crosspoint.xyz[i]);
                    if (rsxyz[i] != xyz[i])
                    {
                        outOfBox = true;
                        break;
                    }
                }
                if (outOfBox)
                    continue;
                if (rs.t < ans.t)
                {
                    ans = rs;
                }
            }
        }
        if (ans.isIntersec)
        {
            return ans;
        }
        //e 的sign
        int esign[3];
        for (int i = 0; i < 3; i++)
        {
            esign[i] = e.xyz[i] < 0 ? -1 : 1;
            if (fabs(e.xyz[i]) < eps)
            {
                esign[i] = 0;
            }
        }
        //e与sign是否一致
        int same[3];
        for (int i = 0; i < 3; i++)
        {
            same[i] = esign[i]*sign[i] >0 ? 1:0;
            if (esign[i] == 0 && sign[i] > 0)
                same[i] = 1;
        }
        for (int i = 0; i < 3; i++)
        {
            xyz[i] = xyz[i] + same[i] * sign[i];
        }
        e = e - vec(sign[0] * same[0], sign[1] * same[1], sign[2] * same[2]);
        e = e + abc;
    }
    return ans;
}
*/
intersec_result Myobj::closet_ray_intersect(vec ro, vec rd, triangle rotri)
{
    vec xyz0 = (ro - vec(xyzmm[0][0], xyzmm[1][0], xyzmm[2][0])) * (1.0 / gridCellWidth);
    vec abc = rd;

    int xyz[3];
    for (int i = 0; i < 3; i++)
    {
        xyz[i] = floor(xyz0.xyz[i]);
    }

    int sign[3];
    for (int i = 0; i < 3; i++)
    {
        sign[i] = abc.xyz[i] < 0 ? -1 : 1;
        if (fabs(abc.xyz[i]) < eps)
        {
            sign[i] = 0;
        }
    }
    
    //ts:触及对应维度网格边界的下一个t值。若sign[i]为0，不考虑那一维。
    int nxyz[3];
    double ts[3];
    for (int i = 0; i < 3; i++)
    {
        if (sign[i])
        {
            //与最近的整数很接近
            if (fabs(floor(xyz0.xyz[i]) - xyz0.xyz[i]) < eps)
            {
                nxyz[i] = xyz[i] + sign[i];
            }
            else
            {
                if (sign[i] > 0)
                {
                    nxyz[i] = (int)ceil(xyz0.xyz[i]);
                }
                else
                {
                    nxyz[i] = (int)floor(xyz0.xyz[i]);
                }
            }
            ts[i] = (nxyz[i] - xyz0.xyz[i]) / abc.xyz[i];
        }
        else
        {
            nxyz[i] = -1;
            ts[i] = DBL_MAX;
        }
    }

    //整个场景的boundingbox边长
    double len[3];
    for (int i = 0; i < 3; i++)
    {
        len[i] = xyzmm[i][1] - xyzmm[i][0];
    }

    //求交结果暂存
    std::map<triangle, intersec_result> intersecMap;

    intersec_result ans(false, 0, 0, 0, 0, DBL_MAX);

    bool reachbound = false;
    while (1)
    {
        for (int i = 0; i < 3; i++)
        {
            //越过网格边界
            if (xyz[i] < 0 || xyz[i] > (int)floor(len[i] / gridCellWidth) + 2)
            {
                reachbound = true;
                break;
            }
        }
        if (reachbound)
            break;
        //对当前xyz格子光线求交
        for (auto& tri : triangleOfGrid[xyz[0]][xyz[1]][xyz[2]])
        {
            //防止自相交
            if (tri == rotri)
                continue;

            if (intersecMap.find(tri) == intersecMap.end())
            {
                intersecMap[tri] = intersect_with_triangle(ro, rd, tri.s, tri.f);
            }
            intersec_result rs = intersecMap[tri];
            if (rs.isIntersec)
            {
                bool outOfBox = false;
                //交点所在格子编号
                vec crosspoint = (ro + (rd * rs.t) - vec(xyzmm[0][0], xyzmm[1][0], xyzmm[2][0])) * (1.0 / gridCellWidth);
                int rsxyz[3];
                for (int i = 0; i < 3; i++)
                {
                    rsxyz[i] = (int)floor(crosspoint.xyz[i]);
                    if (rsxyz[i] != xyz[i])
                    {
                        outOfBox = true;
                        break;
                    }
                }
                if (outOfBox)
                    continue;
                if (rs.t < ans.t)
                {
                    ans = rs;
                }
            }
        }
        if (ans.isIntersec)
        {
            return ans;
        }
        double t = DBL_MAX;
        int ind = -1;
        for (int i = 0; i < 3; i++)
        {
            if (ts[i] < t)
            {
                ind = i;
                t = ts[i];
            }
        }
        if (t == DBL_MAX)
        {
            printf("460!!!!\n");
        }
        for (int i = 0; i < 3; i++)
        {
            if (fabs(t - ts[i]) < eps)
            {
                xyz[i] += sign[i];
                nxyz[i] += sign[i];
                ts[i] = (nxyz[i] - xyz0.xyz[i]) / abc.xyz[i];
            }
        }
    }
    return ans;
}

intersec_result Myobj::closet_ray_intersect_light_triangle(vec ro, vec rd, triangle rotri, std::map<triangle, RadianceRGB >& islight)
{
    vec xyz0 = (ro - vec(xyzmm[0][0], xyzmm[1][0], xyzmm[2][0])) * (1.0 / gridCellWidth);
    vec abc = rd;

    int xyz[3];
    for (int i = 0; i < 3; i++)
    {
        xyz[i] = floor(xyz0.xyz[i]);
    }

    int sign[3];
    for (int i = 0; i < 3; i++)
    {
        sign[i] = abc.xyz[i] < 0 ? -1 : 1;
        if (fabs(abc.xyz[i]) < eps)
        {
            sign[i] = 0;
        }
    }

    //ts:触及对应维度网格边界的下一个t值。若sign[i]为0，不考虑那一维。
    int nxyz[3];
    double ts[3];
    for (int i = 0; i < 3; i++)
    {
        if (sign[i])
        {
            //与最近的整数很接近
            if (fabs(floor(xyz0.xyz[i]) - xyz0.xyz[i]) < eps)
            {
                nxyz[i] = xyz[i] + sign[i];
            }
            else
            {
                if (sign[i] > 0)
                {
                    nxyz[i] = (int)ceil(xyz0.xyz[i]);
                }
                else
                {
                    nxyz[i] = (int)floor(xyz0.xyz[i]);
                }
            }
            ts[i] = (nxyz[i] - xyz0.xyz[i]) / abc.xyz[i];
        }
        else
        {
            nxyz[i] = -1;
            ts[i] = DBL_MAX;
        }
    }

    //整个场景的boundingbox边长
    double len[3];
    for (int i = 0; i < 3; i++)
    {
        len[i] = xyzmm[i][1] - xyzmm[i][0];
    }

    //求交结果暂存
    std::map<triangle, intersec_result> intersecMap;

    intersec_result ans(false, 0, 0, 0, 0, DBL_MAX);

    bool reachbound = false;
    while (1)
    {
        for (int i = 0; i < 3; i++)
        {
            //越过网格边界
            if (xyz[i] < 0 || xyz[i] > (int)floor(len[i] / gridCellWidth) + 2)
            {
                reachbound = true;
                break;
            }
        }
        if (reachbound)
            break;
        //对当前xyz格子光线求交
        for (auto& tri : triangleOfGrid[xyz[0]][xyz[1]][xyz[2]])
        {
            //非光源三角形，跳过
            if (islight.find(tri) == islight.end())
                continue;
            //防止自相交
            if (tri == rotri)
                continue;

            if (intersecMap.find(tri) == intersecMap.end())
            {
                intersecMap[tri] = intersect_with_triangle(ro, rd, tri.s, tri.f);
            }
            intersec_result rs = intersecMap[tri];
            if (rs.isIntersec)
            {
                bool outOfBox = false;
                //交点所在格子编号
                vec crosspoint = (ro + (rd * rs.t) - vec(xyzmm[0][0], xyzmm[1][0], xyzmm[2][0])) * (1.0 / gridCellWidth);
                int rsxyz[3];
                for (int i = 0; i < 3; i++)
                {
                    rsxyz[i] = (int)floor(crosspoint.xyz[i]);
                    if (rsxyz[i] != xyz[i])
                    {
                        outOfBox = true;
                        break;
                    }
                }
                if (outOfBox)
                    continue;
                if (rs.t < ans.t)
                {
                    ans = rs;
                }
            }
        }
        if (ans.isIntersec)
        {
            return ans;
        }
        double t = DBL_MAX;
        int ind = -1;
        for (int i = 0; i < 3; i++)
        {
            if (ts[i] < t)
            {
                ind = i;
                t = ts[i];
            }
        }
        if (t == DBL_MAX)
        {
            printf("460!!!!\n");
        }
        for (int i = 0; i < 3; i++)
        {
            if (fabs(t - ts[i]) < eps)
            {
                xyz[i] += sign[i];
                nxyz[i] += sign[i];
                ts[i] = (nxyz[i] - xyz0.xyz[i]) / abc.xyz[i];
            }
        }
    }
    return ans;
}


//返回指定facet的三个顶点向量。序号为f的facet 属于序号为s的shape
std::array<vec, 3> Myobj::get_vertexes_of_facet(size_t s, size_t f)
{
    triangle tri(s, f);
    if (triangleVertex.find(tri) != triangleVertex.end())
    {
        return triangleVertex.at(tri);
    }

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
    triangleVertex[tri] = points;
    return points;
}

//返回指定facet的三个顶点法向向量。序号为f的facet 属于序号为s的shape
std::array<vec, 3> Myobj::get_normals_of_facet(size_t s, size_t f)
{
    triangle tri(s, f);
    if (triangleVertexNormal.find(tri) != triangleVertexNormal.end())
    {
        return triangleVertexNormal.at(tri);
    }


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
    triangleVertexNormal[tri] = normals;
    return normals;
}

vec Myobj::get_unique_normal_of_facet(size_t s, size_t f)
{
    triangle tri = triangle(s, f);
    if (triangleUniqueNormal.find(tri) != triangleUniqueNormal.end())
    {
        return triangleUniqueNormal.at(tri);
    }
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
    //return n;
    if (w > wr)
    {
        triangleUniqueNormal[tri] = n;
        return n;
    }
    triangleUniqueNormal[tri] = nr;
    return nr;
}
