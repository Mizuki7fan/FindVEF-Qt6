#include "RenderInterface.h"

// 根据编译选项包含相应的渲染器头文件
#ifdef USE_OPENGL1_0
#include "../opengl1/OpenGLRenderer.h"
#endif

#ifdef USE_OPENGL3_0
#include "../opengl3/OpenGL3Renderer.h"
#endif

#ifdef USE_VULKAN
#include "../vulkan/VulkanRenderer.h"
#endif

std::unique_ptr<RenderInterface>
RendererFactory::createRenderer(RenderInterface::RendererType type) {
  switch (type) {
#ifdef USE_OPENGL1_0
  case RenderInterface::OPENGL:
    return std::make_unique<OpenGLRenderer>();
#endif

#ifdef USE_OPENGL3_0
  case RenderInterface::OPENGL3:
    return std::make_unique<OpenGL3Renderer>();
#endif

#ifdef USE_VULKAN
  case RenderInterface::VULKAN:
    return std::make_unique<VulkanRenderer>();
#endif

  default:
    return nullptr;
  }
}
