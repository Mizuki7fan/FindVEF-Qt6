# OpenGL 1.0 渲染后端配置

# 依赖
find_package(OpenGL REQUIRED)

# 源文件收集
file(GLOB OPENGL1_FILES
    ${CMAKE_SOURCE_DIR}/src/render/opengl1/*.cpp
    ${CMAKE_SOURCE_DIR}/src/render/opengl1/*.h
)

# 合并到总渲染文件列表
list(APPEND RENDER_FILES ${OPENGL1_FILES})

# 编译宏
add_definitions(-DUSE_OPENGL1_0)

# 需要追加的链接库
list(APPEND RENDER_LINK_LIBRARIES
    Qt6::OpenGL Qt6::OpenGLWidgets
    OpenGL::GLU OpenGL::GL
)
