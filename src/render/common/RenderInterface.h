#pragma once

#include "../../core/geometry/surfacemesh.h"
#include <memory>
#include <string>

// 前向声明
class FindVEFHandler;

/**
 * @brief 渲染接口抽象基类
 *
 * 定义了渲染系统的通用接口，支持OpenGL和Vulkan两种实现
 */
class RenderInterface {
public:
  virtual ~RenderInterface() = default;

  // 渲染初始化和清理
  virtual void initialize() = 0;
  virtual void cleanup() = 0;
  virtual void resize(int width, int height) = 0;

  // 场景设置
  virtual void setScenePosition(const dop::Point3II &center, float radius) = 0;
  virtual void updateProjectionMatrix() = 0;
  virtual void updateModelViewMatrix() = 0;

  // 变换操作
  virtual void translate(const dop::Vec3II &trans) = 0;
  virtual void rotate(const dop::Vec3II &axis, double angle) = 0;

  // 绘制操作
  virtual void beginRender() = 0;
  virtual void endRender() = 0;
  virtual void clear() = 0;

  // 网格绘制
  virtual void drawMesh(const dop::SM_with_Meta &mesh) = 0;
  virtual void drawPoints() = 0;
  virtual void drawWireframe() = 0;
  virtual void drawFlat() = 0;
  virtual void drawFlatWithNormal() = 0;

  // FindVEF相关绘制
  virtual void drawFindVEF(const FindVEFHandler &handler) = 0;

  // 光照设置
  enum LightingMode {
    LIGHTING_DEFAULT,
    LIGHTING_SOFT,
    LIGHTING_LEGACY,
    LIGHTING_BRIGHT
  };
  virtual void setLightingMode(LightingMode mode) = 0;

  // 材质设置
  virtual void setMaterial(int materialId) = 0;

  // 获取渲染器类型
  enum RendererType { OPENGL, OPENGL3, VULKAN };
  virtual RendererType getRendererType() const = 0;
};

/**
 * @brief 渲染器工厂类
 */
class RendererFactory {
public:
  static std::unique_ptr<RenderInterface>
  createRenderer(RenderInterface::RendererType type);
};