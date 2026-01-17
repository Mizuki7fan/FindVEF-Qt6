#include "operation.h"
#include "../../utils/FVAssert.h"
#include <CGAL/Polygon_mesh_processing/connected_components.h>

namespace dop::MeshOperation {
std::vector<CGAL::SM_Vertex_index>
GetFaceVertices(const SurfaceMesh3 &mesh, const CGAL::SM_Face_index &face) {
  std::vector<CGAL::SM_Vertex_index> face_vertices;
  face_vertices.reserve(6);
  CGAL::SM_Halfedge_index halfedge = mesh.halfedge(face);
  CGAL::SM_Halfedge_index start_halfedge = halfedge;
  do {
    // 使用target而不是source，以匹配CGAL的locate_in_face返回的重心坐标顺序
    face_vertices.push_back(mesh.target(halfedge));
    halfedge = mesh.next(halfedge);
  } while (halfedge != start_halfedge);
  return face_vertices;
}

std::size_t GetNumCpnts(dop::SurfaceMesh3 &mesh) {
  std::vector<std::size_t> map_face_to_cpnt_idx(mesh.num_faces());
  auto map_face_to_component_idx_property = boost::make_iterator_property_map(
      map_face_to_cpnt_idx.begin(),
      boost::typed_identity_property_map<CGAL::SM_Face_index>());
  std::size_t num_cpnts = CGAL::Polygon_mesh_processing::connected_components(
      mesh, map_face_to_component_idx_property);
  std::unordered_map<std::size_t, std::size_t> map_cpnt_idx_to_num_faces;
  for (std::size_t i = 0; i < map_face_to_cpnt_idx.size(); ++i)
    map_cpnt_idx_to_num_faces[map_face_to_cpnt_idx[i]]++;
  for (auto it = map_cpnt_idx_to_num_faces.begin();
       it != map_cpnt_idx_to_num_faces.end(); ++it)
    std::cout << "cpnt_idx: " << it->first << ", num_faces: " << it->second
              << std::endl;

  return num_cpnts;
}

std::array<std::size_t, 2> GetNumBorder(dop::SurfaceMesh3 &mesh) {
  std::vector<std::vector<CGAL::SM_Halfedge_index>> boundary_loops;
  std::vector<std::size_t> map_halfedge_to_visited_flag(mesh.num_halfedges(),
                                                        0);
  for (auto halfedge : mesh.halfedges()) {
    if (!mesh.is_border(halfedge))
      continue;
    if (map_halfedge_to_visited_flag[halfedge.idx()] == 1)
      continue;
    boundary_loops.push_back(std::vector<CGAL::SM_Halfedge_index>());
    auto &loop = boundary_loops.back();
    loop.reserve(mesh.num_halfedges());
    auto curr_halfedge = halfedge;
    do {
      loop.push_back(curr_halfedge);
      map_halfedge_to_visited_flag[curr_halfedge.idx()] = 1;
      curr_halfedge = mesh.next(curr_halfedge);
    } while (curr_halfedge != halfedge);
  }

  std::size_t num_border_edges = 0;
  for (const auto &loop : boundary_loops)
    num_border_edges += loop.size();
  return {boundary_loops.size(), num_border_edges};
}

bool BuildSurfaceMesh(const std::vector<dop::Point3II> vertices,         //
                      const std::vector<std::vector<std::size_t>> faces, //
                      dop::SurfaceMesh3 &mesh) {
  std::vector<int> dummy_mapping;
  return BuildSurfaceMesh(vertices, faces, mesh, dummy_mapping);
}

bool BuildSurfaceMesh(const std::vector<dop::Point3II> vertices,         //
                      const std::vector<std::vector<std::size_t>> faces, //
                      dop::SurfaceMesh3 &mesh,                           //
                      std::vector<int> &map_face_idx_to_manifold_face_idx) {
  if (vertices.empty() || faces.empty())
    return false;

  // 添加顶点到网格
  for (const auto &vertex : vertices)
    mesh.add_vertex(vertex);

  // 添加面到网格
  map_face_idx_to_manifold_face_idx.clear();
  map_face_idx_to_manifold_face_idx.resize(faces.size(), -1);
  int manifold_face_idx = 0;
  for (std::size_t i = 0; i < faces.size(); ++i) {
    const auto &face_vertices = faces[i];
    FV_ASSERT(face_vertices.size() >= 3);
    std::vector<CGAL::SM_Vertex_index> vertex_indices;
    for (std::size_t idx : face_vertices) {
      FV_ASSERT(idx < vertices.size());
      vertex_indices.push_back(CGAL::SM_Vertex_index(static_cast<int>(idx)));
    }
    auto face = mesh.add_face(vertex_indices);
    if (face == mesh.null_face()) {

      map_face_idx_to_manifold_face_idx[i] = -1; // 标记该面为非流形面
    } else {
      map_face_idx_to_manifold_face_idx[i] = manifold_face_idx;
      ++manifold_face_idx;
    }
  }
  return true;
}
} // namespace dop::MeshOperation