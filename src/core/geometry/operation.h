#pragma once
#include "surfacemesh.h"

namespace dop::MeshOperation {
std::vector<CGAL::SM_Vertex_index>
GetFaceVertices(const SurfaceMesh3 &mesh, const CGAL::SM_Face_index &face);

std::size_t GetNumCpnts(dop::SurfaceMesh3 &mesh);

std::array<std::size_t, 2> GetNumBorder(dop::SurfaceMesh3 &mesh);

bool BuildSurfaceMesh(const std::vector<dop::Point3II> vertices,         //
                      const std::vector<std::vector<std::size_t>> faces, //
                      dop::SurfaceMesh3 &mesh);

bool BuildSurfaceMesh(const std::vector<dop::Point3II> vertices,         //
                      const std::vector<std::vector<std::size_t>> faces, //
                      dop::SurfaceMesh3 &mesh,                           //
                      std::vector<int> &map_face_idx_to_manifold_face_idx);
} // namespace dop::MeshOperation