#include "LoadSurfaceMesh.h"
#include "../../../utils/FVAssert.h"
#include "../io/FbxLoader.h"
#include "../io/GltfLoader.h"
#include "../metadata.h"
#include "../operation.h"
#include "../surfacemesh.h"

namespace dop::MeshIO {

void BuildMeshAndRecordNonManifoldFace(
    const std::vector<dop::Point3II> &vertices,                //
    const std::vector<std::vector<std::size_t>> &face_indices, //
    std::vector<int> &map_face_idx_to_manifold_face_idx,       //
    SM_with_Meta &mesh_with_meta) {
  SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  Metadata &meta = mesh_with_meta.metadata;

  map_face_idx_to_manifold_face_idx.clear();
  // 构建网格
  MeshOperation::BuildSurfaceMesh(vertices, face_indices, mesh,
                                  map_face_idx_to_manifold_face_idx);

  //  记录非流形面
  for (std::size_t i = 0; i < map_face_idx_to_manifold_face_idx.size(); ++i) {
    if (map_face_idx_to_manifold_face_idx[i] == -1) {
      const auto &face_vertex_indices = face_indices[i];
      std::vector<std::array<double, 3>> face_points;
      for (std::size_t idx : face_vertex_indices)
        face_points.push_back(
            {vertices[idx][0], vertices[idx][1], vertices[idx][2]});
      meta.AddNonManifoldFace(face_points);
    }
  }

  // 计算网格有多少个连通体
  meta.SetNumCPnts(dop::MeshOperation::GetNumCpnts(mesh));
  meta.SetNumBorders(dop::MeshOperation::GetNumBorder(mesh));
}

bool LoadOFF(const fs::path &filename, SM_with_Meta &mesh_with_meta) {
  SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  Metadata &meta = mesh_with_meta.metadata;
  mesh.clear();
  meta.Init(filename);

  std::ifstream file(filename);
  if (!file.is_open())
    return false;

  std::vector<dop::Point3II> vertices;
  std::vector<std::vector<std::size_t>> face_indices; // 顶点索引

  bool result = CGAL::IO::read_OFF(file, vertices, face_indices);
  if (!result)
    return false;

  std::vector<int> map_face_idx_to_manifold_face_idx;
  BuildMeshAndRecordNonManifoldFace(vertices,                          //
                                    face_indices,                      //
                                    map_face_idx_to_manifold_face_idx, //
                                    mesh_with_meta);
  return true;
}

bool LoadOBJ(const fs::path &filename, SM_with_Meta &mesh_with_meta) {
  SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  Metadata &meta = mesh_with_meta.metadata;
  mesh.clear();
  meta.Init(filename);

  // 手动解析OBJ文件以支持半边法向量（面顶点法向量）
  std::ifstream file(filename);
  if (!file.is_open())
    return false;

  std::vector<dop::Point3II> vertices;

  std::vector<dop::Vec3II> normal_array;
  std::vector<dop::Point2II> texture_array;

  std::vector<std::vector<std::size_t>> face_indices; // 顶点索引
  std::vector<std::vector<std::size_t>> normal_indices;
  std::vector<std::vector<std::size_t>> texture_indices; // 纹理坐标索引

  std::string line;
  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string prefix;
    iss >> prefix;

    if (prefix == "v") {
      // 解析顶点
      double x, y, z;
      iss >> x >> y >> z;
      vertices.emplace_back(x, y, z);
    } else if (prefix == "vt") {
      // 解析纹理坐标
      double u, v;
      iss >> u >> v;
      texture_array.emplace_back(u, v);
    } else if (prefix == "vn") {
      // 解析法向量, obj格式默认为半边法向
      double nx, ny, nz;
      iss >> nx >> ny >> nz;
      normal_array.emplace_back(nx, ny, nz);
    } else if (prefix == "f") {
      // 解析面（支持 v/vt/vn 格式）
      std::vector<std::size_t> face_vertices;
      std::vector<std::size_t> face_normals;
      std::vector<std::size_t> face_textures;

      std::string vertex_data;
      while (iss >> vertex_data) {
        std::istringstream vertex_iss(vertex_data);
        std::string v_str, vt_str, vn_str;

        // 解析 v/vt/vn 格式
        if (std::getline(vertex_iss, v_str, '/')) {
          face_vertices.push_back(std::stoi(v_str) - 1); // OBJ索引从1开始
          if (std::getline(vertex_iss, vt_str, '/')) {
            if (!vt_str.empty())
              face_textures.push_back(std::stoi(vt_str) - 1);
            if (std::getline(vertex_iss, vn_str)) {
              if (!vn_str.empty())
                face_normals.push_back(std::stoi(vn_str) - 1);
            }
          }
        }
      }

      FV_ASSERT(face_vertices.size() >= 3);
      face_indices.push_back(face_vertices);
      if (face_normals.size() == face_vertices.size())
        normal_indices.push_back(face_normals);
      if (face_textures.size() == face_vertices.size())
        texture_indices.push_back(face_textures);
    }
  }
  file.close();

  std::vector<int> map_face_idx_to_manifold_face_idx;
  BuildMeshAndRecordNonManifoldFace(vertices,                          //
                                    face_indices,                      //
                                    map_face_idx_to_manifold_face_idx, //
                                    mesh_with_meta);

  // 处理法线信息，统一存储为fv:normal格式
  if (!normal_array.empty()) {
    meta.SetNormalMappingMode(Metadata::ByHalfedge);
    auto face_vertex_normal =
        mesh.add_property_map<CGAL::SM_Face_index,
                              std::unordered_map<std::size_t, dop::Vec3II>>(
                "fv:normal")
            .first;

    for (CGAL::SM_Face_index face : mesh.faces()) {
      int manifold_face_idx = map_face_idx_to_manifold_face_idx[face.idx()];
      if (manifold_face_idx == -1)
        continue;

      FV_ASSERT_MSG(face.idx() < normal_indices.size(), "面索引越界");
      const auto &face_normal_indices = normal_indices[face.idx()];
      const auto &face_vertex_indices = face_indices[face.idx()];

      // 将每个顶点的法线存储到fv:normal中
      for (std::size_t i = 0;
           i < face_vertex_indices.size() && i < face_normal_indices.size();
           ++i) {
        std::size_t vertex_idx = face_vertex_indices[i];
        std::size_t normal_idx = face_normal_indices[i];
        FV_ASSERT_MSG(normal_idx < normal_array.size(),
                      "法向量索引超出法向量数组范围");
        face_vertex_normal[face][vertex_idx] = normal_array[normal_idx];
      }
    }
  }

  // 处理纹理坐标信息，统一存储为fv:texcoord格式
  if (!texture_array.empty() && !texture_indices.empty()) {
    meta.SetTextureMappingMode(Metadata::ByHalfedge);
    auto face_vertex_texture =
        mesh.add_property_map<CGAL::SM_Face_index,
                              std::unordered_map<std::size_t, dop::Point2II>>(
                "fv:texcoord")
            .first;

    for (CGAL::SM_Face_index face : mesh.faces()) {
      int manifold_face_idx = map_face_idx_to_manifold_face_idx[face.idx()];
      if (manifold_face_idx == -1)
        continue;

      FV_ASSERT_MSG(face.idx() < texture_indices.size(), "面索引越界");
      const auto &face_texture_indices = texture_indices[face.idx()];
      const auto &face_vertex_indices = face_indices[face.idx()];

      // 将每个顶点的纹理坐标存储到fv:texcoord中
      for (std::size_t i = 0;
           i < face_vertex_indices.size() && i < face_texture_indices.size();
           ++i) {
        std::size_t vertex_idx = face_vertex_indices[i];
        std::size_t texture_idx = face_texture_indices[i];
        FV_ASSERT_MSG(texture_idx < texture_array.size(),
                      "纹理坐标索引超出纹理数组范围");
        face_vertex_texture[face][vertex_idx] = texture_array[texture_idx];
      }
    }
  }
  return true;
}

bool SaveOBJ(const fs::path &filename,     //
             SM_with_Meta &mesh_with_meta, //
             bool triangulate) {
  std::ofstream ofs(filename);
  dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  std::vector<std::vector<std::size_t>> faces;

  for (auto p : mesh.points())
    ofs << std::format("v {} {} {}", p[0], p[1], p[2]) << std::endl;

  for (auto face : mesh.faces()) {
    std::vector<CGAL::SM_Vertex_index> face_vertices =
        MeshOperation::GetFaceVertices(mesh, face);

    if (triangulate) {
      for (std::size_t i = 1; i < face_vertices.size() - 1; ++i) {
        ofs << std::format("f {} {} {}", face_vertices[0].idx() + 1,
                           face_vertices[i].idx() + 1,
                           face_vertices[i + 1].idx() + 1)
            << std::endl;
      }
    } else {
      ofs << "f";
      for (auto vertex : face_vertices)
        ofs << " " << vertex.idx() + 1;
      ofs << std::endl;
    }
  }

  ofs.close();

  return true;
}

bool SaveOFF(const fs::path &filename,     //
             SM_with_Meta &mesh_with_meta, //
             bool triangulate) {
  std::ofstream ofs(filename);
  dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  std::vector<std::vector<std::size_t>> faces;

  ofs << "OFF" << std::endl;
  ofs << std::format("{} {} 0", mesh.num_vertices(), mesh.num_faces())
      << std::endl;
  for (auto p : mesh.points())
    ofs << std::format("{} {} {}", p[0], p[1], p[2]) << std::endl;

  for (auto face : mesh.faces()) {
    std::vector<CGAL::SM_Vertex_index> face_vertices =
        MeshOperation::GetFaceVertices(mesh, face);

    if (triangulate) {
      for (std::size_t i = 1; i < face_vertices.size() - 1; ++i) {
        ofs << std::format("3 {} {} {}", face_vertices[0].idx(),
                           face_vertices[i].idx(), face_vertices[i + 1].idx())
            << std::endl;
      }
    } else {
      ofs << face_vertices.size();
      for (auto vertex : face_vertices)
        ofs << " " << vertex.idx();
      ofs << std::endl;
    }
  }

  return true;
}

bool LoadVTK(const fs::path &filename, SM_with_Meta &mesh_with_meta) {
  (void)filename;
  (void)mesh_with_meta;
  return true;
}

bool LoadPLY(const fs::path &filename, SM_with_Meta &mesh_with_meta) {
  SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  Metadata &meta = mesh_with_meta.metadata;
  mesh.clear();
  meta.Init(filename);

  std::ifstream file(filename);
  if (!file.is_open())
    return false;

  std::vector<dop::Point3II> vertices;
  std::vector<std::vector<std::size_t>> face_indices; // 顶点索引

  bool result = CGAL::IO::read_PLY(file, vertices, face_indices);
  if (!result)
    return false;
  std::vector<int> map_face_idx_to_manifold_face_idx;
  BuildMeshAndRecordNonManifoldFace(vertices,                          //
                                    face_indices,                      //
                                    map_face_idx_to_manifold_face_idx, //
                                    mesh_with_meta);
  return true;
}

bool LoadGLTF(const fs::path &filename, SM_with_Meta &mesh_with_meta) {
  SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  Metadata &meta = mesh_with_meta.metadata;
  mesh.clear();
  meta.Init(filename);

  std::vector<dop::Point3II> vertices;
  std::vector<std::vector<std::size_t>> face_indices; // 顶点索引
  bool is_glb = meta.GetFormat() == Metadata::GLB;
  bool result = is_glb ? LoadGltfFile(filename, true, vertices, face_indices)
                       : LoadGltfFile(filename, false, vertices, face_indices);
  if (!result)
    return false;
  std::vector<int> map_face_idx_to_manifold_face_idx;
  BuildMeshAndRecordNonManifoldFace(vertices,                          //
                                    face_indices,                      //
                                    map_face_idx_to_manifold_face_idx, //
                                    mesh_with_meta);

  return true;
}

#ifdef SUPPORT_FBX
bool LoadFbx(const fs::path &filename, SM_with_Meta &mesh_with_meta) {
  SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  Metadata &meta = mesh_with_meta.metadata;
  mesh.clear();
  meta.Init(filename);

  LoadFbxFile(filename, mesh_with_meta);
  // 计算网格有多少个连通体
  meta.SetNumCPnts(dop::MeshOperation::GetNumCpnts(mesh));
  meta.SetNumBorders(dop::MeshOperation::GetNumBorder(mesh));
  return true;
}
#endif
} // namespace dop::MeshIO
