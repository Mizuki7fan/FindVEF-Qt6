# FBX SDK 配置文件
# 此文件包含所有与 FBX SDK 相关的配置
# 注意：此文件只有在 SUPPORT_FBX 为 ON 时才会被包含

message(STATUS "=== Configuring FBX SDK ===")
message(STATUS "FBX support enabled, FBXSDK_PATH: ${FBXSDK_PATH}")

# 设置库路径（使用 paths.cmake 中的路径）
set(FBXSDK_LIB_PATH "${FBXSDK_PATH}/lib/x64")

# FBX相关源文件已经包含在CORE_FILES中
# src/core/io/FbxLoader.h 和 src/core/io/FbxLoader.cpp 会通过CORE_FILES自动包含


# 配置 FBX SDK 的包含目录、编译定义和链接库
# 注意：此函数需要在 add_executable 之后调用
function(configure_fbx_target target_name)
    # 使用 target_* 命令替代全局命令，更好的CMake实践
    target_include_directories(${target_name} PRIVATE "${FBXSDK_PATH}/include")
    target_compile_definitions(${target_name} PRIVATE SUPPORT_FBX)
    target_link_libraries(${target_name} PRIVATE 
        optimized "${FBXSDK_LIB_PATH}/release/libfbxsdk.lib"
        debug "${FBXSDK_LIB_PATH}/debug/libfbxsdk.lib"
    )
endfunction()

# 配置 FBX SDK DLL 复制的函数
function(configure_fbx_dll_copy target_name)
    add_custom_command(TARGET ${target_name}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${FBXSDK_LIB_PATH}/release"
        "$<TARGET_FILE_DIR:${target_name}>"
        COMMENT "Copying FBX SDK DLLs from release directory"
    )
endfunction()

message(STATUS "✓ FBX SDK configuration successful!")
message(STATUS "  FBX_INCLUDE_DIR: ${FBXSDK_PATH}/include")
message(STATUS "  FBX_LIB_PATH: ${FBXSDK_LIB_PATH}")
message(STATUS "=== FBX SDK Configuration Complete ===")
