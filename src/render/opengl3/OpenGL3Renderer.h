#pragma once
#include "../common/RenderInterface.h"
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_0>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>

class OpenGL3Renderer : public RenderInterface, protected QOpenGLFunctions_3_0 {
public:
  OpenGL3Renderer();
  ~OpenGL3Renderer() override;

  // RenderInterface implementation
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

  RendererType getRendererType() const override { return OPENGL3; }

private:
  void setupShaders();
  void setupBuffers();

  QOpenGLShaderProgram *m_shaderProgram;
  QOpenGLBuffer m_vertexBuffer;
  QOpenGLBuffer m_indexBuffer;
  QOpenGLVertexArrayObject m_vao;

  int m_width, m_height;
  LightingMode m_lightingMode;
  int m_currentMaterial;
};