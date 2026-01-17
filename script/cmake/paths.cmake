# 设置项目用到的各种库

set (3rdlib $ENV{OneDrive}/3rd-lib)

set (BOOST_PATH ${3rdlib}/Boost/1.89.0-portable/windows)
set (CGAL_PATH ${3rdlib}/CGAL/CGAL-6.0.1)
set (EIGEN_PATH ${3rdlib}/Eigen/eigen-3.4.0)
set (TRIANGLE_PATH ${3rdlib}/triangle)
set (Qt6_Path ${3rdlib}/Qt/6.8.1-portable)

if (SUPPORT_FBX)
    set (FBXSDK_PATH ${3rdlib}/fbxsdk/2020.3.7)
endif ()

set (gltf_PATH ${3rdlib}/tinygltf)