#include "ModelLoader.h"
#include <array>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <sstream>

namespace {
    using namespace RunaEngine;
    std::string Trim(const std::string& value) {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos) { return {}; }
        auto text = value.substr(first, value.find_last_not_of(" \t\r\n") - first + 1);
        if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
            text = text.substr(1, text.size() - 2);
        }
        return text;
    }
    size_t ResolveIndex(const std::string& text, size_t count) {
        int64_t raw = 0;
        const auto parsed = std::from_chars(text.data(), text.data() + text.size(), raw);
        if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size() || raw == 0) { return count; }
        const int64_t index = raw > 0 ? raw - 1 : static_cast<int64_t>(count) + raw;
        if (index < 0 || index >= static_cast<int64_t>(count)) { return count; }
        return static_cast<size_t>(index);
    }
    bool ReadVector(std::istringstream& stream, Vector3& value) {
        return bool(stream >> value.x >> value.y >> value.z);
    }
    Vector3 Cross(const Vector3& a, const Vector3& b) {
        return {a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x};
    }
    // 凹多角形にも対応するため、面を2Dに投影し、内側にある三角形から切り出す。
    std::vector<std::array<size_t, 3>> Triangulate(const std::vector<VertexData>& face) {
        if (face.size() < 3) { return {}; }
        Vector3 normal{};
        for (size_t i = 0; i < face.size(); ++i) {
            const auto& a = face[i].position;
            const auto& b = face[(i + 1) % face.size()].position;
            normal.x += (a.y-b.y)*(a.z+b.z);
            normal.y += (a.z-b.z)*(a.x+b.x);
            normal.z += (a.x-b.x)*(a.y+b.y);
        }
        const int drop = std::abs(normal.x) >= std::abs(normal.y) && std::abs(normal.x) >= std::abs(normal.z)
            ? 0 : (std::abs(normal.y) >= std::abs(normal.z) ? 1 : 2);
        std::vector<std::array<double, 2>> points;
        for (const auto& vertex : face) {
            const auto& p = vertex.position;
            points.push_back(drop == 0 ? std::array<double, 2>{p.y,p.z} :
                (drop == 1 ? std::array<double, 2>{p.x,p.z} : std::array<double, 2>{p.x,p.y}));
        }
        auto turn = [&](size_t a, size_t b, size_t c) {
            return (points[b][0]-points[a][0])*(points[c][1]-points[a][1]) -
                (points[b][1]-points[a][1])*(points[c][0]-points[a][0]);
        };
        double area = 0;
        double extent = 0;
        for (size_t i = 0; i < points.size(); ++i) {
            const auto& a = points[i]; const auto& b = points[(i+1)%points.size()];
            area += (a[0]-points[0][0])*(b[1]-points[0][1]) - (b[0]-points[0][0])*(a[1]-points[0][1]);
            extent = (std::max)(extent, (std::max)(std::abs(a[0]-points[0][0]), std::abs(a[1]-points[0][1])));
        }
        const double epsilon = (std::max)(extent*extent*1e-10, 1e-30);
        if (std::abs(area) <= epsilon) { return {}; }
        const double sign = area > 0 ? 1 : -1;
        std::vector<size_t> remaining(face.size());
        std::iota(remaining.begin(), remaining.end(), 0);
        std::vector<std::array<size_t, 3>> triangles;
        while (remaining.size() > 3) {
            bool clipped = false;
            for (size_t i = 0; i < remaining.size(); ++i) {
                const size_t a = remaining[(i+remaining.size()-1)%remaining.size()];
                const size_t b = remaining[i]; const size_t c = remaining[(i+1)%remaining.size()];
                if (sign*turn(a,b,c) <= epsilon) { continue; }
                bool containsPoint = false;
                for (const size_t p : remaining) {
                    if (p != a && p != b && p != c && sign*turn(a,b,p) >= -epsilon &&
                        sign*turn(b,c,p) >= -epsilon && sign*turn(c,a,p) >= -epsilon) { containsPoint = true; break; }
                }
                if (!containsPoint) {
                    triangles.push_back({c,b,a}); // X反転で裏返った面の頂点順を戻す
                    remaining.erase(remaining.begin()+static_cast<std::ptrdiff_t>(i));
                    clipped = true; break;
                }
            }
            if (!clipped) { return {}; }
        }
        if (sign*turn(remaining[0],remaining[1],remaining[2]) <= epsilon) { return {}; }
        triangles.push_back({remaining[2],remaining[1],remaining[0]});
        return triangles;
    }
}

RunaEngine::ModelData ModelLoader::LoadObjFile(const std::string& directory, const std::string& filename) {
    using namespace RunaEngine;
    const auto path = std::filesystem::path(directory) / filename;
    std::ifstream file(path);
    if (!file.is_open()) { return {}; }
    ModelData data;
    std::vector<Vector4> positions;
    std::vector<Vector2> uvs;
    std::vector<Vector3> normals;
    std::string meshName = "default", materialName, line;

    while (std::getline(file, line)) {
        line = line.substr(0, line.find('#'));
        std::istringstream stream(line);
        std::string type; stream >> type;
        if (type == "v") {
            Vector3 p{}; if (!ReadVector(stream,p)) { return {}; }
            positions.push_back({-p.x,p.y,p.z,1});
        } else if (type == "vt") {
            Vector2 uv{};
            if (!(stream >> uv.x >> uv.y)) { return {}; }
            uvs.push_back({uv.x,1-uv.y});
        } else if (type == "vn") {
            Vector3 n{}; if (!ReadVector(stream,n)) { return {}; } n.x = -n.x;
            if (n.Length() == 0) { return {}; }
            const auto length=n.Length(); n={n.x/length,n.y/length,n.z/length}; normals.push_back(n);
        } else if (type == "o" || type == "g") {
            std::getline(stream,meshName); meshName=Trim(meshName);
            if (meshName.empty()) { meshName="default"; }
            if (!data.meshes.empty() && !data.meshes.back().subMeshes.empty()) { data.meshes.push_back({meshName,{}}); }
            else if (!data.meshes.empty()) { data.meshes.back().name=meshName; }
        } else if (type == "usemtl") {
            std::getline(stream,materialName); materialName=Trim(materialName);
            if (materialName.empty()) { return {}; }
        } else if (type == "mtllib") {
            std::string mtl; std::getline(stream,mtl); mtl=Trim(mtl);
            if (mtl.empty()) { return {}; }
            // 空白を含む単一のパスを優先。存在しなければ複数のMTL名として読む。
            std::vector<std::string> files;
            if (std::filesystem::exists(path.parent_path()/mtl)) { files.push_back(mtl); }
            else { std::istringstream names(mtl); while(names>>mtl) { files.push_back(mtl); } }
            for(const auto& name:files) {
                const auto mtlPath=path.parent_path()/name;
                auto materials=LoadMaterialTemplateFile(mtlPath.parent_path().generic_string(),mtlPath.filename().generic_string());
                for(auto& entry:materials) { data.materials.insert_or_assign(entry.first,std::move(entry.second)); }
            }
        } else if (type == "f") {
            std::vector<VertexData> face;
            std::vector<bool> missingNormals;
            std::string token;
            while(stream>>token) {
                std::array<std::string,3> parts{};
                std::istringstream definition(token);
                for(auto& part:parts) { std::getline(definition,part,'/'); }
                std::string excess;
                if (std::getline(definition,excess,'/')) { return {}; }
                VertexData vertex{};
                const size_t positionIndex = ResolveIndex(parts[0], positions.size());
                if (positionIndex == positions.size()) { return {}; }
                vertex.position=positions[positionIndex];
                if (!parts[1].empty()) {
                    const size_t uvIndex = ResolveIndex(parts[1], uvs.size());
                    if (uvIndex == uvs.size()) { return {}; }
                    vertex.texCoord=uvs[uvIndex];
                }
                if (!parts[2].empty()) {
                    const size_t normalIndex = ResolveIndex(parts[2], normals.size());
                    if (normalIndex == normals.size()) { return {}; }
                    vertex.normal=normals[normalIndex];
                }
                missingNormals.push_back(parts[2].empty()); face.push_back(vertex);
            }
            const auto triangles=Triangulate(face);
            if (triangles.empty()) { return {}; }
            if (data.vertices.size()+triangles.size()*3 > UINT32_MAX) { return {}; }
            if(data.meshes.empty()) { data.meshes.push_back({meshName,{}}); }
            auto& mesh=data.meshes.back();
            if(mesh.subMeshes.empty() || mesh.subMeshes.back().materialName!=materialName) {
                mesh.subMeshes.push_back({materialName,static_cast<uint32_t>(data.vertices.size()),0});
            }
            for(const auto& triangle:triangles) {
                const auto& a=face[triangle[0]].position;
                const auto& b=face[triangle[1]].position;
                const auto& c=face[triangle[2]].position;
                auto normal=Cross({b.x-a.x,b.y-a.y,b.z-a.z},{c.x-a.x,c.y-a.y,c.z-a.z});
                const float length=normal.Length();
                if (length == 0) { return {}; }
                normal={normal.x/length,normal.y/length,normal.z/length};
                for(const size_t index:triangle) {
                    auto vertex=face[index];
                    if(missingNormals[index]) { vertex.normal=normal; }
                    data.vertices.push_back(vertex);
                }
                mesh.subMeshes.back().vertexCount+=3;
            }
        }
    }
    std::erase_if(data.meshes,[](const auto& mesh){return mesh.subMeshes.empty();});
    if (data.vertices.empty()) { return {}; }

    return data;
}

std::unordered_map<std::string,RunaEngine::MaterialData> ModelLoader::LoadMaterialTemplateFile(
    const std::string& directory,const std::string& filename) {
    const auto path=std::filesystem::path(directory)/filename;
    std::ifstream file(path);
    if (!file.is_open()) { return {}; }
    std::unordered_map<std::string,RunaEngine::MaterialData> materials;
    RunaEngine::MaterialData* current=nullptr;
    std::string line;

    while(std::getline(file,line)) {
        line=line.substr(0,line.find('#'));
        std::istringstream stream(line); std::string type; stream>>type;
        if(type=="newmtl") {
            std::string name; std::getline(stream,name); name=Trim(name);
            if (name.empty()) { return {}; } current=&materials[name];
        } else if(current && type=="Kd") {
            RunaEngine::Vector3 color{}; if (!ReadVector(stream,color)) { return {}; }
            current->color.x=color.x; current->color.y=color.y; current->color.z=color.z;
        } else if(current && type=="Ks") { if (!ReadVector(stream,current->specular)) { return {}; }
        } else if(current && type=="Ns") {
            if (!(stream>>current->shininess)) { return {}; }
        } else if(current && (type=="d" || type=="Tr")) {
            float alpha=0;
            if (!(stream>>alpha)) { return {}; }
            current->color.w=type=="d"?alpha:1-alpha;
        } else if(current && type=="map_Kd") {
            std::string name; std::getline(stream,name); name=Trim(name);
            if (name.empty() || name.front()=='-') { return {}; }
            current->textureFilePath=(path.parent_path()/name).lexically_normal().generic_string();
        }
    }
    return materials;
}
