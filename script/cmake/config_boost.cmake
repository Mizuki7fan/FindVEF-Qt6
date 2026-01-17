# find_boost.cmake - Boost 库查找和配置
# 此文件专门处理 Boost 库的查找、配置和链接

message(STATUS "=== Configuring Boost Library ===")

# 使用 paths.cmake 中定义的 BOOST_PATH
if(DEFINED BOOST_PATH AND NOT "${BOOST_PATH}" STREQUAL "")
    set(BOOST_ROOT "${BOOST_PATH}")
    message(STATUS "BOOST_PATH: ${BOOST_PATH}")
else()
    message(WARNING "BOOST_PATH not defined in paths.cmake, will search in system paths")
endif()

# CMP = CMake Policy
# CMP0167是专门处理 Boost 1.70+ 库查找方式的策略, 优先使用BoostConfig.cmake模式
# 这里将其设置为OLD, 使用传统的FindBoost.cmake模块, 增加兼容性
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)  # 使用旧策略以获得更好的兼容性
endif()

# 多层次 Boost 查找策略
set(Boost_FOUND FALSE)

# 方法1: 如果指定了路径且路径存在，尝试手动设置
if(DEFINED BOOST_ROOT AND EXISTS "${BOOST_ROOT}")
    message(STATUS "Boost path exists: ${BOOST_ROOT}")
    
    # 检查常见的目录结构
    if(EXISTS "${BOOST_ROOT}/lib")
        set(BOOST_LIBRARYDIR "${BOOST_ROOT}/lib")
        message(STATUS "Found Boost lib directory: ${BOOST_LIBRARYDIR}")
    elseif(EXISTS "${BOOST_ROOT}/stage/lib")
        set(BOOST_LIBRARYDIR "${BOOST_ROOT}/stage/lib")
        message(STATUS "Found Boost stage/lib directory: ${BOOST_LIBRARYDIR}")
    endif()
    
    if(EXISTS "${BOOST_ROOT}/include")
        set(BOOST_INCLUDEDIR "${BOOST_ROOT}/include")
        message(STATUS "Found Boost include directory: ${BOOST_INCLUDEDIR}")
    endif()
    
    # 尝试 CONFIG 模式查找
    find_package(Boost QUIET CONFIG COMPONENTS filesystem)
    if(Boost_FOUND)
        message(STATUS "Found Boost using CONFIG mode with specified path")
    endif()
endif()

# 方法2: 如果 CONFIG 模式失败，尝试传统的 Module 模式
if(NOT Boost_FOUND)
    message(STATUS "Trying Boost Module mode...")
    find_package(Boost QUIET COMPONENTS filesystem)
    if(Boost_FOUND)
        message(STATUS "Found Boost using Module mode")
    endif()
endif()

# 方法3: 如果还是失败，尝试不指定组件
if(NOT Boost_FOUND)
    message(STATUS "Trying to find Boost without specific components...")
    find_package(Boost QUIET)
    if(Boost_FOUND)
        message(STATUS "Found Boost without specific components")
        # 尝试手动查找 filesystem 库
        find_library(Boost_FILESYSTEM_LIBRARY 
            NAMES boost_filesystem libboost_filesystem
            PATHS ${BOOST_LIBRARYDIR} ${BOOST_ROOT}/lib ${BOOST_ROOT}/stage/lib
            NO_DEFAULT_PATH
        )
        if(Boost_FILESYSTEM_LIBRARY)
            message(STATUS "Found Boost filesystem library: ${Boost_FILESYSTEM_LIBRARY}")
        endif()
    endif()
endif()

# 最终检查和错误处理
if(Boost_FOUND)
    message(STATUS "✓ Boost configuration successful!")
    message(STATUS "  Boost_VERSION: ${Boost_VERSION}")
    message(STATUS "  Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "  Boost_LIBRARIES: ${Boost_LIBRARIES}")
    
    # 设置配置成功标志
    # 注意：使用 include() 时，所有变量都在同一作用域中，无需 PARENT_SCOPE
    # find_package 已自动设置 Boost_FOUND, Boost_INCLUDE_DIRS, Boost_LIBRARIES, Boost_VERSION
    set(BOOST_CONFIGURED TRUE)
    
else()
    message(FATAL_ERROR 
        "✗ Could not find Boost library!\n"
        "Please check the following:\n"
        "1. Verify BOOST_PATH in script/cmake/paths.cmake points to correct Boost installation\n"
        "2. Ensure Boost is properly installed with filesystem component\n"
        "3. Check that the directory structure contains 'include' and 'lib' (or 'stage/lib') folders\n"
        "Current BOOST_PATH: ${BOOST_PATH}\n"
        "Current BOOST_ROOT: ${BOOST_ROOT}"
    )
endif()

message(STATUS "=== Boost Configuration Complete ===")