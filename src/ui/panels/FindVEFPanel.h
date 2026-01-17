#pragma once

// 根据编译选项选择合适的面板实现
#ifdef USE_OPENGL3_0
#include "../../render/opengl3/OpenGL3FindVEFPanel.h"
using FindVEFPanel = OpenGL3FindVEFPanel;
#elif defined(USE_VULKAN)
// Vulkan 面板实现（待实现）
#include "../../render/common/BasePanel.h"
using FindVEFPanel = BasePanel; // 临时使用基类
#else
// 默认使用 OpenGL 1.0
#include "../../render/opengl1/OpenGLFindVEFPanel.h"
using FindVEFPanel = OpenGLFindVEFPanel;
#endif