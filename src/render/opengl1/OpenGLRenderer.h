#pragma once

#include "../common/RenderInterface.h"
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <array>

// 前向声明
class QOpenGLTexture;

/**
 * @brief OpenGL渲染器实现
 *
 * 基于OpenGL 3.0+的渲染实现
 */
class OpenGLRenderer : public RenderInterface, public QOpenGLFunctions {
public:
  OpenGLRenderer();
  virtual ~OpenGLRenderer();

  // 实现RenderInterface接口
  void initialize() override;
  void cleanup() override;
  void resize(int width, int height) override;

  void setScenePosition(const dop::Point3II &center, float radius) override;
  void updateProjectionMatrix() override;
  void updateModelViewMatrix() override;

  void translate(const dop::Vec3II &trans) override;
  void rotate(const dop::Vec3II &axis, double angle) override;

  void beginRender() override;
  void endRender() override;
  void clear() override;

  void drawMesh(const dop::SM_with_Meta &mesh) override;
  void drawPoints() override;
  void drawWireframe() override;
  void drawFlat() override;
  void drawFlatWithNormal() override;

  void drawFindVEF(const FindVEFHandler &handler) override;

  void setLightingMode(LightingMode mode) override;
  void setMaterial(int materialId) override;

  RendererType getRendererType() const override { return OPENGL; }

private:
  // OpenGL特定的私有方法
  void setupLighting();
  void setupMaterial(int materialId);
  void applyLightingMode(LightingMode mode);

  // 相机配置
  struct CameraConfiguration {
    dop::Point3II Center;
    double Radius;
    dop::Vec3II TotalTrans;
    std::array<double, 16> ProjectionMatrix;
    std::array<double, 16> ModelViewMatrix;
  };
  CameraConfiguration camera;

  // 渲染状态
  LightingMode currentLightingMode = LIGHTING_DEFAULT;
  int currentMaterialId = 0;
  int viewportWidth = 800;
  int viewportHeight = 600;
};