# Qt6 库查找和配置
# 此文件负责查找和配置Qt6库及其组件

message(STATUS "=== Configuring Qt Library ===")

# 设置Qt6路径, 这里的Qt6_Path是paths.cmake中设置的变量
set(Qt6_DIR "${Qt6_Path}/lib/cmake/Qt6")

# 添加Qt6的前缀路径，帮助CMake找到所有Qt6组件
list(APPEND CMAKE_PREFIX_PATH "${Qt6_Path}")

# 输出调试信息
message(STATUS "Qt6_DIR: ${Qt6_DIR}")

# 查找Qt6包及其组件
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui OpenGL OpenGLWidgets)

if(Qt6_FOUND)
    message(STATUS "✓ Qt6 configuration successful! Version: ${Qt6_VERSION}")
    message(STATUS "  Version: ${Qt6_VERSION}")
else()
    message(FATAL_ERROR "未找到Qt6库，请检查Qt6_Path配置")
endif()

# 标记Qt6已配置
set(QT6_CONFIGURED TRUE)

message(STATUS "=== Qt6 Configuration Complete ===")

# 配置Qt6目标属性的函数
function(configure_qt6_target target_name)
    message(STATUS "Configuring Qt6 target properties for: ${target_name}")
    
    set_target_properties(${target_name} PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
    
    message(STATUS "✓ Qt6 target properties configured successfully!")
endfunction()

# 配置Qt6部署的函数
function(configure_qt6_deployment target_name)
    message(STATUS "Configuring Qt6 deployment for: ${target_name}")
    
    # 查找windeployqt的位置并通过该文件复制项目的依赖库
    if (CMAKE_SYSTEM_NAME MATCHES "Windows")
        # 直接使用paths.cmake中定义的Qt路径
        set(_qt_bin_dir "${Qt6_Path}/bin")    
        if(EXISTS "${_qt_bin_dir}")
            find_program(DEPLOYQT_EXECUTABLE NAMES windeployqt HINTS "${_qt_bin_dir}")
            if (DEPLOYQT_EXECUTABLE)
                message(STATUS "Found windeployqt.exe at: ${DEPLOYQT_EXECUTABLE}")
                add_custom_command(TARGET ${target_name}
                    POST_BUILD
                    COMMAND ${DEPLOYQT_EXECUTABLE} "$<TARGET_FILE:${target_name}>"
                    --$<LOWER_CASE:$<CONFIG>> --qmldir ${CMAKE_SOURCE_DIR}
                    COMMENT "Deploying Qt libraries"
                )
                message(STATUS "✓ Qt6 deployment configured successfully!")
            else ()
                message (WARNING "没有在${_qt_bin_dir}目录找到windeployqt.exe，跳过Qt库部署")
            endif()
        else()
            message(WARNING "Qt bin directory does not exist: ${_qt_bin_dir}")
        endif()
    else()
        message(STATUS "Non-Windows platform, skipping windeployqt configuration")
    endif()
endfunction()
