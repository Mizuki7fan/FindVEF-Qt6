# 定义 SUPPORT_FBX 选项
option(SUPPORT_FBX "Enable support for FBX format" ON)

# 设置渲染模式
option(USE_OPENGL1_0 "Enable OpenGL 1.0 rendering" ON)
option(USE_OPENGL3_0 "Enable OpenGL 3.0 rendering" OFF)
option(USE_VULKAN "Enable Vulkan rendering" OFF)

if(NOT USE_OPENGL1_0 AND NOT USE_OPENGL3_0 AND NOT USE_VULKAN)
    message(FATAL_ERROR "请至少设置一种渲染模式USE_OPENGL1_0/USE_OPENGL3_0/USE_VULKAN")
endif()
