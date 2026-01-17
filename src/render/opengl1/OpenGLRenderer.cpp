#include "OpenGLRenderer.h"
#include "../../utils/FindVEFHandler.h"

// Windows平台需要在包含OpenGL头文件之前包含windows.h
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <GL/glu.h>
#include <QOpenGLTexture>

OpenGLRenderer::OpenGLRenderer() {
  // 初始化相机配置
  camera.Center = dop::Point3II(0, 0, 0);
  camera.Radius = 1.0;
  camera.TotalTrans = dop::Vec3II(0, 0, 0);
}

OpenGLRenderer::~OpenGLRenderer() { cleanup(); }

void OpenGLRenderer::initialize() {
  initializeOpenGLFunctions();

  // 启用深度测试
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  // 启用面剔除
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // 设置清除颜色
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

  // 启用光照
  glEnable(GL_LIGHTING);
  setupLighting();

  // 启用颜色材质
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

  // 启用法线归一化
  glEnable(GL_NORMALIZE);
}

void OpenGLRenderer::cleanup() {
  // OpenGL资源清理
}

void OpenGLRenderer::resize(int width, int height) {
  viewportWidth = width;
  viewportHeight = height;
  glViewport(0, 0, width, height);
  updateProjectionMatrix();
}

void OpenGLRenderer::setScenePosition(const dop::Point3II &center,
                                      float radius) {
  camera.Center = center;
  camera.Radius = radius;
  updateProjectionMatrix();
  updateModelViewMatrix();
}

void OpenGLRenderer::updateProjectionMatrix() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  double aspect = static_cast<double>(viewportWidth) / viewportHeight;
  double fovy = 45.0;
  double nearPlane = 0.01 * camera.Radius;
  double farPlane = 100.0 * camera.Radius;

  gluPerspective(fovy, aspect, nearPlane, farPlane);

  // 保存投影矩阵
  glGetDoublev(GL_PROJECTION_MATRIX, camera.ProjectionMatrix.data());
}

void OpenGLRenderer::updateModelViewMatrix() {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // 设置相机位置
  gluLookAt(0, 0, 3.0 * camera.Radius, camera.Center[0], camera.Center[1],
            camera.Center[2], 0, 1, 0);

  // 应用变换
  glTranslated(camera.TotalTrans[0], camera.TotalTrans[1],
               camera.TotalTrans[2]);

  // 保存模型视图矩阵
  glGetDoublev(GL_MODELVIEW_MATRIX, camera.ModelViewMatrix.data());
}

void OpenGLRenderer::translate(const dop::Vec3II &trans) {
  // CGAL向量不支持直接通过[]赋值，需要创建新向量
  camera.TotalTrans = camera.TotalTrans + trans;
  updateModelViewMatrix();
}

void OpenGLRenderer::rotate(const dop::Vec3II &axis, double angle) {
  glMatrixMode(GL_MODELVIEW);
  glRotated(angle, axis[0], axis[1], axis[2]);
  glGetDoublev(GL_MODELVIEW_MATRIX, camera.ModelViewMatrix.data());
}

void OpenGLRenderer::beginRender() {
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixd(camera.ProjectionMatrix.data());

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(camera.ModelViewMatrix.data());
}

void OpenGLRenderer::endRender() { glFlush(); }

void OpenGLRenderer::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::drawMesh(const dop::SM_with_Meta &mesh) {
  (void)mesh;
  // TODO: 实现网格绘制
  // 这里需要根据具体的网格数据结构来实现
}

void OpenGLRenderer::drawPoints() {
  // TODO: 实现点绘制
}

void OpenGLRenderer::drawWireframe() {
  // TODO: 实现线框绘制
}

void OpenGLRenderer::drawFlat() {
  // TODO: 实现平面绘制
}

void OpenGLRenderer::drawFlatWithNormal() {
  // TODO: 实现带法线的平面绘制
}

void OpenGLRenderer::drawFindVEF(const FindVEFHandler &handler) {
  (void)handler;
  // TODO: 实现FindVEF绘制
}

void OpenGLRenderer::setLightingMode(LightingMode mode) {
  currentLightingMode = mode;
  applyLightingMode(mode);
}

void OpenGLRenderer::setMaterial(int materialId) {
  currentMaterialId = materialId;
  setupMaterial(materialId);
}

void OpenGLRenderer::setupLighting() {
  // 设置默认光照
  GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
  GLfloat light_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
  GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat light_position[] = {1.0f, 1.0f, 1.0f, 0.0f};

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable(GL_LIGHT0);
}

void OpenGLRenderer::setupMaterial(int materialId) {
  (void)materialId;
  // TODO: 根据materialId设置不同的材质属性
}

void OpenGLRenderer::applyLightingMode(LightingMode mode) {
  switch (mode) {
  case LIGHTING_DEFAULT:
    // 默认光照设置
    break;
  case LIGHTING_SOFT:
    // 柔和光照设置
    break;
  case LIGHTING_LEGACY:
    // 传统光照设置
    break;
  case LIGHTING_BRIGHT:
    // 明亮光照设置
    break;
  }
}