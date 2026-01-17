#pragma once

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>

#include "metadata.h"
namespace dop {

using KernelII = CGAL::Simple_cartesian<double>;
typedef KernelII::Point_2 Point2II;
typedef KernelII::Point_3 Point3II;
typedef KernelII::Vector_2 Vec2II;
typedef KernelII::Vector_3 Vec3II;

// 原始的CGAL Surface_mesh类型
typedef CGAL::Surface_mesh<Point3II> SurfaceMesh3;

// 包装类：将网格和元数据封装在一起
class SurfaceMesh3WithMetadata {
public:
  SurfaceMesh3 mesh; // 网格数据
  Metadata metadata; // 元数据

  // 构造函数
  SurfaceMesh3WithMetadata() = default;

  // 提供对mesh的直接访问（为了兼容性）
  SurfaceMesh3 &operator*() { return mesh; }
  const SurfaceMesh3 &operator*() const { return mesh; }
  SurfaceMesh3 *operator->() { return &mesh; }
  const SurfaceMesh3 *operator->() const { return &mesh; }

  bool Empty() const { return mesh.is_empty(); }
  void Clear() {
    mesh.clear();
    metadata.Clear();
  }
};

typedef SurfaceMesh3WithMetadata SM_with_Meta;
}; // namespace dop
