# Vulkan 渲染后端配置

# 依赖
find_package(Vulkan REQUIRED)

# 源文件收集
file(GLOB VULKAN_FILES
    ${CMAKE_SOURCE_DIR}/src/render/vulkan/*.cpp
    ${CMAKE_SOURCE_DIR}/src/render/vulkan/*.h
)

# 合并到总渲染文件列表
list(APPEND RENDER_FILES ${VULKAN_FILES})

# 编译宏
add_definitions(-DUSE_VULKAN)

# 需要追加的链接库
list(APPEND RENDER_LINK_LIBRARIES Vulkan::Vulkan)
