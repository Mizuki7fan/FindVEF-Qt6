#include "OpenGLFindVEFPanel.h"
#include "../../core/geometry/io/LoadSurfaceMesh.h"
#include "../../core/geometry/operation.h"

#include <CGAL/Polygon_mesh_processing/bbox.h>
#include <CGAL/Polygon_mesh_processing/measure.h>

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFontMetrics>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QTextLayout>
#include <QUrl>
#include <QWheelEvent>
#include <algorithm>
#include <cassert>
#include <format>
#include <iostream>
#include <optional>
#include <qobject.h>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

namespace PMP = CGAL::Polygon_mesh_processing;

OpenGLFindVEFPanel::OpenGLFindVEFPanel(MainWindow *parent, int k)
    : BasePanel(parent), parent(parent), panel_id(k) {}

OpenGLFindVEFPanel::~OpenGLFindVEFPanel() {}

void OpenGLFindVEFPanel::clearInfo() {
  mesh_with_meta.Clear();
  findvef_info.Clear();
}

void OpenGLFindVEFPanel::UpdateCamera() {

  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  if (mesh.is_empty())
    return;

  auto bbox = CGAL::Polygon_mesh_processing::bbox(mesh);
  ptMin = dop::Point3II(bbox.xmin(), bbox.ymin(), bbox.zmin());
  ptMax = dop::Point3II(bbox.xmax(), bbox.ymax(), bbox.zmax());

  double avelen = 0.0;
  double maxlen = 0.0;
  double minlen = std::numeric_limits<double>::max();
  for (const auto &eh : mesh.edges()) {
    double len = PMP::edge_length(eh, mesh);
    maxlen = std::max(maxlen, len);
    minlen = std::min(minlen, len);
    avelen += len;
  }

  set_scene_pos(CGAL::midpoint(ptMin, ptMax),
                CGAL::sqrt(CGAL::squared_distance(ptMin, ptMax)) * 0.5);
  ;

  std::cout << "BoundingBox:" << std::endl;
  std::cout << "X: [" << ptMin[0] << ", " << ptMax[0] << "]" << std::endl;
  std::cout << "Y: [" << ptMin[1] << ", " << ptMax[1] << "]" << std::endl;
  std::cout << "Z: [" << ptMin[2] << ", " << ptMax[2] << "]" << std::endl;
  std::cout << "Diag length of BBox: "
            << CGAL::sqrt(CGAL::squared_distance(ptMin, ptMax)) << std::endl;
  std::cout << "Edge Length: [" << minlen << ", " << maxlen
            << "]; AVG: " << avelen / mesh.num_edges() << std::endl;
}

QImage OpenGLFindVEFPanel::SnapShot() {

  update();
  QImage image = grabFramebuffer();
  // 将边缘1像素设置为黑框
  // 获取原图像的宽度和高度
  int width = image.width();
  int height = image.height();

  assert(width >= 1 && height >= 1);

  // 设置边缘1像素为黑色
  for (int x = 0; x < width; ++x) {
    image.setPixel(x, 0, qRgb(0, 0, 0));          // 顶部边缘
    image.setPixel(x, height - 1, qRgb(0, 0, 0)); // 底部边缘
  }

  for (int y = 0; y < height; ++y) {
    image.setPixel(0, y, qRgb(0, 0, 0));         // 左边缘
    image.setPixel(width - 1, y, qRgb(0, 0, 0)); // 右边缘
  }

  return image;
}

void OpenGLFindVEFPanel::jumpToVEF(std::string str) {

  std::cout << "有待实现" << std::endl;
}

void OpenGLFindVEFPanel::initializeGL() { BasePanel::initializeGL(); }

void OpenGLFindVEFPanel::draw() {

  update_draw_option();

  if (EnableLighting)
    glEnable(GL_LIGHTING);

  // 支持多种绘制模式同时启用
  if (DrawPoints)
    draw_points();

  if (DrawFlat)
    draw_flat();

  if (DrawFlatWithNormal)
    draw_flat_with_normal();

  if (DrawNonManifold)
    draw_non_manifold();

  if (DrawWireframe) {
    // 在绘制线框时关闭光照，保证线框可见
    if (glIsEnabled(GL_LIGHTING)) {
      glDisable(GL_LIGHTING);
      draw_wireframe();
      glEnable(GL_LIGHTING);
    } else {
      draw_wireframe();
    }
  }

  // 在绘制FindVEF叠加内容时：临时关闭影响可见性的状态，并为点使用自适应像素大小
  if (DrawFindVEF) {
    GLboolean lighting_was_enabled = glIsEnabled(GL_LIGHTING);
    GLboolean cull_was_enabled = glIsEnabled(GL_CULL_FACE);
    if (lighting_was_enabled)
      glDisable(GL_LIGHTING);
    if (cull_was_enabled)
      glDisable(GL_CULL_FACE);

    // ---- 自适应点大小：基于视口尺寸的像素大小，并做上限/下限夹取 ----
    GLint viewport[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    const GLint vp_w = viewport[2];
    const GLint vp_h = viewport[3];
    const GLint vp_min =
        (vp_w > 0 && vp_h > 0) ? (vp_w < vp_h ? vp_w : vp_h) : 800; // 兜底

    // 以视口较小边的0.6%作为基准，确保在不同窗口/缩放下有稳定可见度
    GLfloat desired_px = (GLfloat)(vp_min * 0.006f);

    // 查询硬件支持的点大小范围并夹取
    GLfloat size_range[2] = {1.0f, 1.0f};
    glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, size_range);
    const GLfloat hw_min = size_range[0];
    const GLfloat hw_max = size_range[1];

    const GLfloat user_min = 3.0f;  // 自定义下限，避免过小
    const GLfloat user_max = 16.0f; // 自定义上限，避免过大
    GLfloat point_px = desired_px;
    if (point_px < user_min)
      point_px = user_min;
    if (point_px > user_max)
      point_px = user_max;
    if (point_px < hw_min)
      point_px = hw_min;
    if (point_px > hw_max)
      point_px = hw_max;

    // 记录旧的点状态并设置新的点大小与平滑
    GLboolean point_smooth_was_enabled = glIsEnabled(GL_POINT_SMOOTH);
    GLfloat old_point_size = 1.0f;
    glGetFloatv(GL_POINT_SIZE, &old_point_size);
    if (!point_smooth_was_enabled)
      glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glPointSize(point_px);

    draw_findvef();

    // 恢复点状态
    glPointSize(old_point_size > 0.0f ? old_point_size : 1.0f);
    if (!point_smooth_was_enabled)
      glDisable(GL_POINT_SMOOTH);

    if (cull_was_enabled)
      glEnable(GL_CULL_FACE);
    if (lighting_was_enabled)
      glEnable(GL_LIGHTING);
  }

  draw_moji();
  if (EnableLighting)
    glDisable(GL_LIGHTING);
  if (DrawBnd)
    draw_bnd_glu();
  if (DrawMeshInfo)
    draw_mesh_info();
}

void OpenGLFindVEFPanel::draw_points(void) const {

  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  if (mesh.is_empty())
    return;
  auto normals =
      mesh.property_map<CGAL::SM_Vertex_index, dop::Vec3II>("v:normal");

  // 设置点的属性使其可见
  glPointSize(5.0f);           // 设置点大小
  glColor3f(1.0f, 0.0f, 0.0f); // 设置为红色

  glBegin(GL_POINTS);

  // 如果有原始顶点信息，使用原始顶点绘制
  for (const auto &vh : mesh.vertices()) {
    glVertex3dv(&mesh.point(vh)[0]);
  }

  glEnd();

  // 恢复默认点大小
  glPointSize(1.0f);
}

void OpenGLFindVEFPanel::draw_flat(void) const {

  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  if (mesh.is_empty())
    return;

  // 启用多边形偏移，确保实体面在正确的深度位置
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f); // 正值使实体面稍微远离观察者

  // 设置默认颜色，确保面可见
  glColor3f(0.8f, 0.8f, 0.8f); // 设置为浅灰色

  // 使用CGAL的面信息（支持任意多边形）
  for (CGAL::SM_Face_index fh : mesh.faces()) {
    // 收集面的所有顶点
    std::vector<CGAL::SM_Vertex_index> face_vertices;
    CGAL::SM_Halfedge_index halfedge = mesh.halfedge(fh);
    CGAL::SM_Halfedge_index start_halfedge = halfedge;

    do {
      face_vertices.push_back(mesh.target(halfedge));
      halfedge = mesh.next(halfedge);
    } while (halfedge != start_halfedge);

    if (face_vertices.size() == 3) {
      // 三角形面
      glBegin(GL_TRIANGLES);
      for (const auto &vertex : face_vertices)
        glVertex3dv(&mesh.point(vertex)[0]);
      glEnd();
    } else if (face_vertices.size() == 4) {
      // 四边形面 - 使用GL_QUADS
      glBegin(GL_QUADS);
      for (const auto &vertex : face_vertices)
        glVertex3dv(&mesh.point(vertex)[0]);
      glEnd();
    } else {
      // 多边形面：使用扇形三角化绘制
      // 注意：这里修复了之前的类型错误
      glBegin(GL_TRIANGLES);
      for (std::size_t i = 1; i < face_vertices.size() - 1; ++i) {
        // 绘制三角形：顶点0, 顶点i, 顶点i+1
        for (std::size_t j : {std::size_t(0), i, i + 1})
          glVertex3dv(&mesh.point(face_vertices[j])[0]);
      }
      glEnd();
      // 恢复默认设置
      glDisable(GL_POLYGON_OFFSET_LINE);
    }
  }

  // 恢复默认设置
  glDisable(GL_POLYGON_OFFSET_FILL);
}

void OpenGLFindVEFPanel::draw_flat_with_normal(void) const {

  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  if (mesh.is_empty())
    return;
  const Metadata &meta = mesh_with_meta.metadata;

  // 使用 fv:normal 属性（face 属性，值为 unordered_map<顶点索引, 法线向量>）
  auto fv_normal_map =
      mesh.property_map<CGAL::SM_Face_index,
                        std::unordered_map<std::size_t, dop::Vec3II>>(
          "fv:normal");

  if (!fv_normal_map.has_value()) {
    draw_flat();
    return;
  }

  const auto &fv_normals = fv_normal_map.value();

  // 设置光源和材质 - 使用当前选择的光照模式
  applyLightingMode(currentLightingMode);
  setDefaultMaterial();

  // 检查光照状态，如果未启用则临时启用（显示法线效果需要光照）
  GLboolean lighting_was_enabled = glIsEnabled(GL_LIGHTING);
  if (!lighting_was_enabled)
    glEnable(GL_LIGHTING);

  // 启用法线归一化
  glEnable(GL_NORMALIZE);
  // 设置基础颜色
  glColor3f(0.8f, 0.8f, 0.8f);

  // 启用多边形偏移，确保实体面在正确的深度位置
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f); // 正值使实体面稍微远离观察者

  // 使用CGAL的面信息（支持任意多边形）
  for (CGAL::SM_Face_index fh : mesh.faces()) {
    // 获取当前面的法线映射
    const auto &face_normal_map = fv_normals[fh];

    // 收集面的所有顶点
    std::vector<CGAL::SM_Vertex_index> face_vertices;
    face_vertices.reserve(6);

    CGAL::SM_Halfedge_index halfedge = mesh.halfedge(fh);
    CGAL::SM_Halfedge_index start_halfedge = halfedge;

    do {
      face_vertices.push_back(mesh.target(halfedge));
      halfedge = mesh.next(halfedge);
    } while (halfedge != start_halfedge);

    if (face_vertices.size() == 3 || face_vertices.size() == 4) {
      glBegin(face_vertices.size() == 3 ? GL_TRIANGLES : GL_QUADS);
      for (std::size_t i = 0; i < face_vertices.size(); ++i) {
        std::size_t vertex_idx = face_vertices[i].idx();

        // 从 fv:normal 的 unordered_map 中获取该顶点的法线
        auto normal_it = face_normal_map.find(vertex_idx);
        if (normal_it != face_normal_map.end()) {
          glNormal3dv(&normal_it->second[0]);
        }

        glVertex3dv(&mesh.point(face_vertices[i])[0]);
      }
      glEnd();
    } else {
      // 多边形面：使用扇形三角化绘制
      glBegin(GL_TRIANGLES);
      for (std::size_t i = 1; i < face_vertices.size() - 1; ++i)
        for (std::size_t j : {std::size_t(0), i, i + 1}) {
          std::size_t vertex_idx = face_vertices[j].idx();

          // 从 fv:normal 的 unordered_map 中获取该顶点的法线
          auto normal_it = face_normal_map.find(vertex_idx);
          if (normal_it != face_normal_map.end()) {
            glNormal3dv(&normal_it->second[0]);
          }

          glVertex3dv(&mesh.point(face_vertices[j])[0]);
        }
      glEnd();
      // 恢复默认设置
      glDisable(GL_POLYGON_OFFSET_LINE);
    }
  }

  // 恢复默认设置
  glDisable(GL_POLYGON_OFFSET_FILL);
  glDisable(GL_NORMALIZE);

  // 恢复光照的原始状态
  if (!lighting_was_enabled)
    glDisable(GL_LIGHTING);
}

void OpenGLFindVEFPanel::draw_wireframe(void) const {

  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  if (mesh.is_empty())
    return;

  // 启用多边形偏移来解决Z-fighting问题
  glEnable(GL_POLYGON_OFFSET_LINE);
  glPolygonOffset(-1.0f, -1.0f); // 负值使线框更靠近观察者

  // 设置线条属性使其可见
  glLineWidth(2.0f);           // 设置线宽
  glColor3f(0.0f, 0.0f, 0.0f); // 设置为黑色，与白色背景形成对比

  glBegin(GL_LINES);
  for (const auto &eh : mesh.edges()) {
    CGAL::SM_Halfedge_index halfedge = mesh.halfedge(eh);
    glVertex3dv(&mesh.point(mesh.source(halfedge))[0]);
    glVertex3dv(&mesh.point(mesh.target(halfedge))[0]);
  }
  glEnd();

  // 恢复默认设置
  glLineWidth(1.0f);
  glDisable(GL_POLYGON_OFFSET_LINE);
}

void OpenGLFindVEFPanel::draw_bnd_glu(void) const {

  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  if (mesh.is_empty())
    return;

  glLineWidth(5.0);
  glColor3f(1.0, 0.0, 0.0);
  glBegin(GL_LINES);

  for (const auto &edge : mesh.edges()) {
    if (mesh.is_border(edge)) {
      CGAL::SM_Halfedge_index halfedge = mesh.halfedge(edge);
      dop::Point3II p0 = mesh.point(mesh.source(halfedge));
      dop::Point3II p1 = mesh.point(mesh.target(halfedge));

      glVertex3dv(&p0[0]);
      glVertex3dv(&p1[0]);
    }
  }

  glEnd();
  glLineWidth(1.0);
  glColor3f(0.0, 0.0, 0.0);
}

void OpenGLFindVEFPanel::draw_findvef(void) const {
  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  if (mesh.is_empty() || findvef_info.IsEmpty())
    return;
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.1f);
  // 启用深度测试，确保绘制的元素正确考虑深度遮挡关系
  glEnable(GL_DEPTH_TEST);
  int point_size = 10; // 固定点大小
  int line_size = 4;   // 固定线宽

  if (findvef_info.vertex.size() != 0) {
    if (DrawFindVEF) { // 使用panel的DrawFindVEF设置
      // 保存光照状态
      GLboolean was_lighting = glIsEnabled(GL_LIGHTING);

      // 绘制纯色点：禁用光照
      if (was_lighting)
        glDisable(GL_LIGHTING);

      // 启用点偏移，确保点在面的前方
      glEnable(GL_POLYGON_OFFSET_POINT);
      glPolygonOffset(-2.0f, -2.0f); // 使用更大的负值，确保点在FindVEF面前方

      glEnable(GL_POINT_SMOOTH);
      glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

      glPointSize(point_size > 3 ? (GLfloat)point_size : 5.0f);
      glBegin(GL_POINTS);
      for (auto A : findvef_info.vertex) {
        if (A.first >= mesh.num_vertices())
          continue;
        // glColor4ub表示unsigned byte，取值范围[0,255]
        glColor4ub(A.second[0], A.second[1], A.second[2], 255);
        CGAL::SM_Vertex_index vh(A.first);
        dop::Point3II p = mesh.point(vh);
        glVertex3dv(&p[0]);
      }
      glEnd();

      // 恢复默认状态
      glPointSize(1.0f);
      glDisable(GL_POINT_SMOOTH);
      glDisable(GL_POLYGON_OFFSET_POINT);

      if (was_lighting)
        glEnable(GL_LIGHTING);
    }
  }

  if (findvef_info.face.size() != 0) {
    if (DrawFindVEF) { // 使用panel的DrawFindVEF设置
      // 保存光照状态
      GLboolean was_lighting = glIsEnabled(GL_LIGHTING);

      // 绘制纯色面：禁用光照
      if (was_lighting)
        glDisable(GL_LIGHTING);

      // 启用多边形偏移，确保面在正确的深度位置
      // 使用负值使FindVEF的面更靠近观察者，避免被模型遮挡
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(-0.25f, -0.25f);

      for (auto A : findvef_info.face) {
        if (A.first >= mesh.num_faces())
          continue;

        // 收集面的所有顶点
        std::vector<CGAL::SM_Vertex_index> face_vertices;
        CGAL::SM_Halfedge_index halfedge =
            mesh.halfedge(CGAL::SM_Face_index(A.first));
        CGAL::SM_Halfedge_index start_halfedge = halfedge;
        do {
          face_vertices.push_back(mesh.target(halfedge));
          halfedge = mesh.next(halfedge);
        } while (halfedge != start_halfedge);

        if (face_vertices.size() < 3)
          continue; // 非法面，跳过

        // 设置颜色（带透明度以便更好地观察）
        glColor4ub(A.second[0], A.second[1], A.second[2], 200);

        // 统一使用扇形三角化绘制，支持任意多边形（包括三角形与四边形）
        glBegin(GL_TRIANGLES);
        for (std::size_t i = 1; i < face_vertices.size() - 1; ++i) {
          for (std::size_t j : {std::size_t(0), i, i + 1})
            glVertex3dv(&mesh.point(face_vertices[j])[0]);
        }
        glEnd();
      }

      // 恢复默认设置
      glDisable(GL_POLYGON_OFFSET_FILL);
      if (was_lighting)
        glEnable(GL_LIGHTING);
    }
  }

  if (findvef_info.point.size() != 0) {

    if (DrawFindVEF) {
      // 保存光照状态
      GLboolean was_lighting = glIsEnabled(GL_LIGHTING);

      // 绘制纯色点：禁用光照
      if (was_lighting)
        glDisable(GL_LIGHTING);

      // 启用点偏移，确保点在面的前方
      glEnable(GL_POLYGON_OFFSET_POINT);
      glPolygonOffset(-1.0f, -1.0f); // 使用更大的负值，确保点在FindVEF面前方

      glEnable(GL_POINT_SMOOTH);
      glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

      glPointSize(point_size > 3 ? (GLfloat)point_size : 5.0f);
      glBegin(GL_POINTS);
      for (auto A : findvef_info.point) {
        glColor4ub(A.second[0], A.second[1], A.second[2], 255);
        glVertex3dv(A.first.data());
      }
      glEnd();

      // 恢复默认状态
      glPointSize(1.0f);
      glDisable(GL_POINT_SMOOTH);
      glDisable(GL_POLYGON_OFFSET_POINT);

      if (was_lighting)
        glEnable(GL_LIGHTING);
    }
  }

  if (findvef_info.point_edge.size() != 0) {
    if (DrawFindVEF) {
      // 启用线偏移，确保线在正确的深度位置
      glEnable(GL_POLYGON_OFFSET_LINE);
      glPolygonOffset(-0.5f, -0.5f); // 负值使线框更靠近观察者

      glLineWidth(line_size);
      glBegin(GL_LINES);

      for (auto A : findvef_info.point_edge) {
        glColor4ub(A.second[0], A.second[1], A.second[2], 255);
        glVertex3dv(A.first.first.data());
        glVertex3dv(A.first.second.data());
      }
      glEnd();
      // 恢复默认设置
      glDisable(GL_POLYGON_OFFSET_LINE);
    }
  }

  if (findvef_info.vertex_edge.size() != 0) {
    if (DrawFindVEF) {
      // 启用线偏移，确保线在正确的深度位置
      glEnable(GL_POLYGON_OFFSET_LINE);
      glPolygonOffset(-0.5f, -0.5f); // 负值使线框更靠近观察者

      glLineWidth(line_size);
      glBegin(GL_LINES);

      for (auto A : findvef_info.vertex_edge) {
        if (A.first.first >= mesh.num_vertices() ||
            A.first.second >= mesh.num_vertices())
          continue;
        glColor4ub(A.second[0], A.second[1], A.second[2], 255);
        glVertex3dv(&mesh.point(CGAL::SM_Vertex_index(A.first.first))[0]);
        glVertex3dv(&mesh.point(CGAL::SM_Vertex_index(A.first.second))[0]);
      }
      glEnd();
      // 恢复默认设置
      glDisable(GL_POLYGON_OFFSET_LINE);
    }
  }

  glDisable(GL_ALPHA_TEST);
}

void OpenGLFindVEFPanel::draw_mesh_info(void) const {
  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  if (mesh.is_empty())
    return;

  // 确保所有OpenGL绘制命令已完成
  glFinish();

  // 获取当前的模型视图矩阵和投影矩阵
  GLdouble modelview[16];
  GLdouble projection[16];
  GLint viewport[4];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
  glGetDoublev(GL_PROJECTION_MATRIX, projection);
  glGetIntegerv(GL_VIEWPORT, viewport);

  // 预先计算所有顶点和面的屏幕坐标
  struct ScreenLabel {
    int x, y;
    QString text;
    QColor color;
  };
  std::vector<ScreenLabel> labels;

  // 计算顶点标签位置
  for (const auto &vh : mesh.vertices()) {
    const dop::Point3II &point = mesh.point(vh);

    // 将 3D 坐标投影到 2D 屏幕坐标
    GLdouble winX, winY, winZ;
    gluProject(point.x(), point.y(), point.z(), modelview, projection, viewport,
               &winX, &winY, &winZ);

    // 检查点是否在视锥体内（winZ 在 0 到 1 之间表示在可见范围内）
    if (winZ < 0.0 || winZ > 1.0)
      continue;

    // OpenGL 的 Y 坐标是从底部开始的，需要转换为 Qt 的坐标系统
    int screenX = static_cast<int>(winX);
    int screenY = viewport[3] - static_cast<int>(winY);

    QString vertexLabel = QString("v%1").arg(vh.idx());
    labels.push_back({screenX + 5, screenY - 5, vertexLabel, Qt::red});
  }

  // 计算面标签位置
  for (const auto &fh : mesh.faces()) {
    // 计算面的中心点
    dop::Point3II center(0, 0, 0);
    int vertex_count = 0;

    CGAL::SM_Halfedge_index halfedge = mesh.halfedge(fh);
    CGAL::SM_Halfedge_index start_halfedge = halfedge;
    do {
      const dop::Point3II &point = mesh.point(mesh.target(halfedge));
      center = dop::Point3II(center.x() + point.x(), center.y() + point.y(),
                             center.z() + point.z());
      vertex_count++;
      halfedge = mesh.next(halfedge);
    } while (halfedge != start_halfedge);

    // 计算平均值得到中心点
    if (vertex_count > 0) {
      center =
          dop::Point3II(center.x() / vertex_count, center.y() / vertex_count,
                        center.z() / vertex_count);
    }

    // 将面中心投影到 2D 屏幕坐标
    GLdouble winX, winY, winZ;
    gluProject(center.x(), center.y(), center.z(), modelview, projection,
               viewport, &winX, &winY, &winZ);

    // 检查点是否在视锥体内
    if (winZ < 0.0 || winZ > 1.0)
      continue;

    int screenX = static_cast<int>(winX);
    int screenY = viewport[3] - static_cast<int>(winY);

    QString faceLabel = QString("f%1").arg(fh.idx());
    labels.push_back({screenX, screenY, faceLabel, Qt::blue});
  }

  // 使用 QPainter 绘制所有预先计算好的标签
  QPainter painter(const_cast<OpenGLFindVEFPanel *>(this));
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::TextAntialiasing);
  painter.setFont(QFont("Arial", 10));

  for (const auto &label : labels) {
    painter.setPen(label.color);
    painter.drawText(label.x, label.y, label.text);
  }

  painter.end();
}

void OpenGLFindVEFPanel::draw_moji(void) {

  const dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;

  const Metadata &meta = mesh_with_meta.metadata;
  if (mesh.is_empty())
    return;
  glPushAttrib(GL_ALL_SHADER_BITS);
  // 获取非流形面数量
  size_t non_manifold_count =
      mesh_with_meta.metadata.GetNonManifoldFaces().size();

  QString mesh_info = QString::fromStdString(std::format(
      "V: {}, F: {}, E: {}, cpnt: {}, border: {}/{}, non_manifold: {}",
      mesh.num_vertices(), mesh.num_faces(), mesh.num_edges(),
      meta.GetNumCPnts(), meta.GetNumBorders()[0], meta.GetNumBorders()[1],
      non_manifold_count));

  QString mesh_name = QString::fromStdString(meta.GetFileNameWithExt());
  QString mesh_full_path = QString::fromStdString(meta.GetFilePath().string());
  QString findvef_name = QString::fromStdString(findvef_info.GetFileName());

  // 新增的元数据信息
  QString file_size =
      QString::fromStdString(std::format("{} bytes", meta.GetFileSize()));
  QString file_format = QString::fromStdString(meta.GetFormatString());
  QString normal_type =
      QString::fromStdString(meta.GetNormalMappingModeString());
  QString texture_type =
      QString::fromStdString(meta.GetTextureMappingModeString());

  QPainter painter(this);
  int windowWidth = this->width();
  int margin = 3;   // 边距
  qreal y = margin; // 初始 y 坐标

  // 在右上角绘制panel编号
  const QString panelNumbers[] = {"①", "②", "③", "④", "⑤", "⑥", "⑦", "⑧"};
  if (panel_id >= 0 && panel_id < 8) {
    painter.setPen(Qt::gray);
    painter.setFont(QFont("Arial", 20, QFont::Bold));
    QFontMetrics fm(painter.font());
    QString panelNumber = panelNumbers[panel_id];
    int textWidth = fm.horizontalAdvance(panelNumber);
    int textHeight = fm.height();
    painter.drawText(windowWidth - textWidth - margin, textHeight, panelNumber);
  }

  auto drawText = [&](const QString &text, int Height) {
    if (!text.isEmpty()) {
      QString textWithBullet = "· " + text;
      QRect textRect(margin, y, windowWidth - 2 * margin, Height);
      QTextLayout textLayout(textWithBullet, painter.font());
      QTextOption textOption;
      textOption.setWrapMode(
          QTextOption::WrapAnywhere); // 设置允许任意字符处换行
      textLayout.setTextOption(textOption);
      textLayout.beginLayout();
      while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
          break;
        line.setLineWidth(textRect.width());
        line.setPosition(QPointF(textRect.left(), y));
        y += line.height();
      }
      textLayout.endLayout();
      textLayout.draw(&painter, QPointF(textRect.left(), textRect.top()));
      y -= 0.5 * Height;
    }
  };

  // 显示文件名/文件完整路径
  if (DrawModelName) {
    painter.setPen(Qt::darkMagenta); // 深紫色
    painter.setFont(QFont("Consolas", 15));
    int height_1 = QFontMetrics(painter.font()).height();

    drawText(panel_name, height_1);
    drawText(mesh_name, height_1);
    drawText(findvef_name, height_1);
  }

  if (DrawModelFullPath) {
    painter.setPen(Qt::darkMagenta); // 深紫色
    painter.setFont(QFont("Consolas", 15));
    int height_1 = QFontMetrics(painter.font()).height();

    drawText(panel_name, height_1);
    drawText(mesh_full_path, height_1);
    drawText(findvef_name, height_1);
  }

  // 设置蓝色小字样式，用于后续的元数据信息
  painter.setPen(Qt::darkBlue);
  painter.setFont(QFont("Consolas", 10));
  int height_small = QFontMetrics(painter.font()).height();

  // 第一排：点线面数（蓝色小字）
  if (DrawInfo) {
    drawText(mesh_info, height_small);
  }

  // 第二排：文件大小/格式（同一行）
  if (DrawFileSizeFormat) {
    QString file_info = QString("File: %1, %2").arg(file_size).arg(file_format);
    drawText(file_info, height_small);
  }

  // 第三排：法线样式（保持现状）
  QString normal_info = QString("Normal: %1").arg(normal_type);
  drawText(normal_info, height_small);

  // 第四排: 纹理样式（保持现状）
  QString texture_info = QString("Texture: %1").arg(texture_type);
  drawText(texture_info, height_small);

  painter.end();
  glPopAttrib();
}

void OpenGLFindVEFPanel::update_draw_option() {

  // 获取当前panel的独立绘制参数
  const PanelDrawSettings &settings =
      parent->wController->getPanelSettings(panel_id);

  // EnableLighting 使用默认值 (在头文件中已设置为 true)

  DrawBnd = settings.drawBnd;
  DrawFindVEF = settings.drawFindVEF;
  DrawModelName = settings.drawModelName;
  DrawModelFullPath = settings.drawModelFullPath;
  DrawInfo = settings.drawInfo;
  DrawFileSizeFormat = settings.drawFileSizeFormat;
  DrawMeshInfo = settings.drawMeshInfo;

  // 更新绘制模式状态

  DrawPoints = settings.drawPoints;
  DrawWireframe = settings.drawWireframe;
  DrawFlat = settings.drawFlat;
  DrawFlatWithNormal = settings.drawFlatWithNormal;
  DrawNonManifold = settings.drawNonManifold; // 新增：非流形面绘制选项
}

void OpenGLFindVEFPanel::dropEvent(QDropEvent *_event) {
  QList<QUrl> urls = _event->mimeData()->urls();
  if (urls.isEmpty())
    return;
  QString file_name_qstring = urls.first().toLocalFile();
  if (file_name_qstring.isEmpty())
    return;

  fs::path file_path(file_name_qstring.toStdString());
  std::string ext = file_path.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == ".findvef") {
    findvef_info.Load(file_path);
  } else if (ext == ".off") {
    clearInfo();
    dop::MeshIO::LoadOFF(file_path, mesh_with_meta);
  } else if (ext == ".obj") {
    clearInfo();
    dop::MeshIO::LoadOBJ(file_path, mesh_with_meta);
  } else if (ext == ".vtk") {
    clearInfo();
    dop::MeshIO::LoadVTK(file_path, mesh_with_meta);
  } else if (ext == ".ply") {
    clearInfo();
    dop::MeshIO::LoadPLY(file_path, mesh_with_meta);
  } else if (ext == ".glb") {
    clearInfo();
    dop::MeshIO::LoadGLTF(file_path, mesh_with_meta);
  } else if (ext == ".gltf") {
    clearInfo();
    dop::MeshIO::LoadGLTF(file_path, mesh_with_meta);
  }

#ifdef SUPPORT_FBX
  else if (ext == ".fbx") {
    clearInfo();
    dop::MeshIO::LoadFbx(file_path, mesh_with_meta);
  }
#endif
  else
    return;

  if (ext != ".findvef") {
    mesh_with_meta.metadata.PrintInfo();
    UpdateCamera();
  }
  updateGL();
  _event->acceptProposedAction();
  return;
}

void OpenGLFindVEFPanel::dragEnterEvent(QDragEnterEvent *_event) {
  if (_event->mimeData()->hasUrls() ||
      _event->mimeData()->hasFormat("text/uri-list")) {
    _event->acceptProposedAction();
  } else {
    _event->ignore();
  }
}

void OpenGLFindVEFPanel::mousePressEvent(QMouseEvent *_event) {

  // 调用基类逻辑，建立trackball起始点与鼠标模式
  BasePanel::mousePressEvent(_event);
  parent->syncCameraConfiguration(panel_id, camera);
}

void OpenGLFindVEFPanel::mouseMoveEvent(QMouseEvent *_event) {

  // 调用基类逻辑，处理旋转/平移并触发重绘
  BasePanel::mouseMoveEvent(_event);
  parent->syncCameraConfiguration(panel_id, camera);
}

void OpenGLFindVEFPanel::mouseReleaseEvent(QMouseEvent *_event) {

  // 调用基类逻辑，重置鼠标模式
  BasePanel::mouseReleaseEvent(_event);
  parent->syncCameraConfiguration(panel_id, camera);
}

void OpenGLFindVEFPanel::wheelEvent(QWheelEvent *_event) {

  // 调用基类逻辑，处理缩放
  BasePanel::wheelEvent(_event);
  parent->syncCameraConfiguration(panel_id, camera);
}

// ==================== 光照模式管理 ====================

void OpenGLFindVEFPanel::setLightingMode(LightingMode mode) {

  currentLightingMode = mode;
  // 立即应用新的光照模式
  makeCurrent();
  applyLightingMode(mode);
  update(); // 重新绘制
}

void OpenGLFindVEFPanel::applyLightingMode(LightingMode mode) const {

  // 先禁用所有光源
  for (int i = 0; i < 8; ++i) {
    glDisable(GL_LIGHT0 + i);
  }

  switch (mode) {
  case LIGHTING_DEFAULT: {
    // 默认改进光照 - 解决阴影问题的主要配置
    GLfloat globalAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // 主光源
    GLfloat pos1[] = {10.0f, 10.0f, 10.0f, 0.0f};
    GLfloat col1_diffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat col1_specular[] = {0.8f, 0.8f, 0.8f, 1.0f};

    // 填充光
    GLfloat pos2[] = {-8.0f, -8.0f, -8.0f, 0.0f};
    GLfloat col2_diffuse[] = {0.4f, 0.4f, 0.4f, 1.0f};

    // 侧光
    GLfloat pos3[] = {15.0f, 0.0f, 0.0f, 0.0f};
    GLfloat col3_diffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};

    // 底部补光
    GLfloat pos4[] = {0.0f, -12.0f, 5.0f, 0.0f};
    GLfloat col4_diffuse[] = {0.3f, 0.3f, 0.3f, 1.0f};

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos1);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col1_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col1_specular);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, pos2);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, col2_diffuse);

    glEnable(GL_LIGHT2);
    glLightfv(GL_LIGHT2, GL_POSITION, pos3);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, col3_diffuse);

    glEnable(GL_LIGHT3);
    glLightfv(GL_LIGHT3, GL_POSITION, pos4);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, col4_diffuse);
    break;
  }

  case LIGHTING_SOFT: {
    // 柔和光照 - 最大程度减少阴影
    GLfloat globalAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f}; // 更高的环境光
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // 使用更多的低强度光源从各个角度照射
    GLfloat positions[][4] = {
        {8.0f, 8.0f, 8.0f, 0.0f},   // 右上前
        {-8.0f, 8.0f, 8.0f, 0.0f},  // 左上前
        {8.0f, -8.0f, 8.0f, 0.0f},  // 右下前
        {-8.0f, -8.0f, 8.0f, 0.0f}, // 左下前
        {0.0f, 0.0f, -10.0f, 0.0f}, // 后方
        {0.0f, 12.0f, 0.0f, 0.0f}   // 顶部
    };

    GLfloat softColor[] = {0.35f, 0.35f, 0.35f, 1.0f};

    for (int i = 0; i < 6; ++i) {
      glEnable(GL_LIGHT0 + i);
      glLightfv(GL_LIGHT0 + i, GL_POSITION, positions[i]);
      glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, softColor);
    }
    break;
  }

  case LIGHTING_LEGACY: {
    // 传统光照 - 原始配置（用于对比）
    GLfloat globalAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    GLfloat pos[] = {0.0f, 0.0f, 10.0f, 0.0f};
    GLfloat col[] = {0.8f, 0.8f, 0.8f, 1.0f};

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);
    break;
  }

  case LIGHTING_BRIGHT: {
    // 明亮光照 - 高环境光，适合查看细节
    GLfloat globalAmbient[] = {0.6f, 0.6f, 0.6f, 1.0f}; // 很高的环境光
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // 主光源强度降低，避免过曝
    GLfloat pos1[] = {10.0f, 10.0f, 10.0f, 0.0f};
    GLfloat col1[] = {0.4f, 0.4f, 0.4f, 1.0f};

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos1);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col1);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col1);
    break;
  }
  }
}

void OpenGLFindVEFPanel::draw_non_manifold(void) const {

  const auto &faces = mesh_with_meta.metadata.GetNonManifoldFaces();

  if (faces.size() == 0)
    return;

  // 绘制非流形面的面 - 使用浅黄色
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f); // 正值使实体面稍微远离观察者

  glColor3f(0.6f, 0.8f, 1.0f); // 设置为浅蓝色
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  for (const auto &face : faces) {
    if (face.size() == 3 || face.size() == 4) {
      glBegin(face.size() == 3 ? GL_TRIANGLES : GL_POLYGON);
      for (const auto &point : face)
        glVertex3d(point[0], point[1], point[2]);
      glEnd();
    } else {
      glBegin(GL_TRIANGLES);
      // 使用扇形三角化绘制
      for (std::size_t i = 1; i < face.size() - 1; ++i)
        for (std::size_t j : {std::size_t(0), i, i + 1})
          glVertex3d(face[j][0], face[j][1], face[j][2]);
      glEnd();
      // 恢复默认设置
      glDisable(GL_POLYGON_OFFSET_LINE);
    }
  }

  glDisable(GL_POLYGON_OFFSET_FILL);

  // 绘制线框 - 使用加粗的黑色，确保线框显示在面的前方
  glEnable(GL_POLYGON_OFFSET_LINE);
  glPolygonOffset(-1.0f, -1.0f); // 负值使线框更靠近观察者

  glLineWidth(3.0f);           // 设置为加粗线宽
  glColor3f(0.0f, 0.0f, 0.0f); // 设置为黑色
  glBegin(GL_LINES);

  for (const auto &face : faces) {
    for (std::size_t i = 0; i < face.size(); ++i) {
      const auto &p1 = face[i];
      const auto &p2 = face[(i + 1) % face.size()];
      glVertex3d(p1[0], p1[1], p1[2]);
      glVertex3d(p2[0], p2[1], p2[2]);
    }
  }

  glEnd();
  glLineWidth(1.0f); // 恢复默认线宽
  glDisable(GL_POLYGON_OFFSET_LINE);
}

void OpenGLFindVEFPanel::ExportMesh(std::string format, bool triangulate) {
  if (mesh_with_meta.mesh.is_empty())
    return;

  // 生成默认文件名
  QString defaultFileName, fileName;
  if (triangulate)
    defaultFileName = QString::fromStdString(std::format(
        "{}_tri.{}", mesh_with_meta.metadata.GetFileName(), format));
  else
    defaultFileName = QString::fromStdString(
        std::format("{}.{}", mesh_with_meta.metadata.GetFileName(), format));

  if (format == "obj") {
    fileName = QFileDialog::getSaveFileName(this, "Export OBJ", defaultFileName,
                                            "OBJ Files (*.obj)");
    dop::MeshIO::SaveOBJ(fileName.toStdString(), mesh_with_meta, triangulate);
  } else if (format == "off") {
    fileName = QFileDialog::getSaveFileName(this, "Export OFF", defaultFileName,
                                            "OFF Files (*.off)");
    dop::MeshIO::SaveOFF(fileName.toStdString(), mesh_with_meta, triangulate);
  }
}