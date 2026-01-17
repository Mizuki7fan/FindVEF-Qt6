#include "GltfLoader.h"
#include "../metadata.h"
#include "../operation.h"

// 定义实现宏
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <iostream>

bool LoadGltfFile(const fs::path &filename,             //
                  bool is_glb,                          //
                  std::vector<dop::Point3II> &vertices, //
                  std::vector<std::vector<std::size_t>> &face_indices) {
  vertices.clear();
  face_indices.clear();

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  // 加载GLB文件
  bool ret =
      is_glb ? loader.LoadBinaryFromFile(&model, &err, &warn, filename.string())
             : loader.LoadASCIIFromFile(&model, &err, &warn, filename.string());

  if (!ret) {
    std::cerr << "Failed to load GLB file: " << filename << std::endl;
    return false;
  }

  // 检查是否有网格数据
  if (model.meshes.empty()) {
    std::cerr << "No mesh data found in GLB file: " << filename << std::endl;
    return false;
  }

  // 遍历所有网格
  for (const auto &gltf_mesh : model.meshes) {
    for (const auto &primitive : gltf_mesh.primitives) {
      // glTF 格式是为了 GPU 高效渲染而设计的，因此它只支持底层图形 API（如
      // OpenGL、Vulkan）直接支持的图元类型。
      if (primitive.mode != TINYGLTF_MODE_TRIANGLES) {
        continue;
      }

      // 获取顶点位置数据
      const auto &positions_accessor =
          model.accessors[primitive.attributes.at("POSITION")];
      const auto &positions_view =
          model.bufferViews[positions_accessor.bufferView];
      const auto &positions_buffer = model.buffers[positions_view.buffer];

      // 获取数据指针和步幅
      const unsigned char *data_ptr = positions_buffer.data.data() +
                                      positions_view.byteOffset +
                                      positions_accessor.byteOffset;
      int byte_stride = positions_accessor.ByteStride(positions_view);
      if (byte_stride == 0) {
        // 如果 stride 为 0，说明数据是紧密排列的
        // 根据 glTF 规范，POSITION 属性通常是 VEC3 FLOAT
        byte_stride = sizeof(float) * 3;
      }

      // 将顶点添加到vertices向量
      std::size_t base_vertex_index = vertices.size();
      for (std::size_t i = 0; i < positions_accessor.count; ++i) {
        const float *pos =
            reinterpret_cast<const float *>(data_ptr + i * byte_stride);
        vertices.emplace_back(pos[0], pos[1], pos[2]);
      }

      // 检查是否存在索引
      if (primitive.indices > -1) {
        // 获取索引数据
        const auto &indices_accessor = model.accessors[primitive.indices];
        const auto &indices_view =
            model.bufferViews[indices_accessor.bufferView];
        const auto &indices_buffer = model.buffers[indices_view.buffer];

        // 根据索引数据类型处理
        if (indices_accessor.componentType ==
            TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
          const uint16_t *indices = reinterpret_cast<const uint16_t *>(
              &indices_buffer.data[indices_view.byteOffset +
                                   indices_accessor.byteOffset]);
          for (std::size_t i = 0; i < indices_accessor.count; i += 3) {
            if (i + 2 < indices_accessor.count) {
              face_indices.push_back(
                  {base_vertex_index + static_cast<std::size_t>(indices[i]),
                   base_vertex_index + static_cast<std::size_t>(indices[i + 1]),
                   base_vertex_index +
                       static_cast<std::size_t>(indices[i + 2])});
            }
          }
        } else if (indices_accessor.componentType ==
                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
          const uint32_t *indices = reinterpret_cast<const uint32_t *>(
              &indices_buffer.data[indices_view.byteOffset +
                                   indices_accessor.byteOffset]);
          for (std::size_t i = 0; i < indices_accessor.count; i += 3) {
            if (i + 2 < indices_accessor.count) {
              face_indices.push_back(
                  {base_vertex_index + static_cast<std::size_t>(indices[i]),
                   base_vertex_index + static_cast<std::size_t>(indices[i + 1]),
                   base_vertex_index +
                       static_cast<std::size_t>(indices[i + 2])});
            }
          }
        } else if (indices_accessor.componentType ==
                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
          const uint8_t *indices = reinterpret_cast<const uint8_t *>(
              &indices_buffer.data[indices_view.byteOffset +
                                   indices_accessor.byteOffset]);
          for (std::size_t i = 0; i < indices_accessor.count; i += 3) {
            if (i + 2 < indices_accessor.count) {
              face_indices.push_back(
                  {base_vertex_index + static_cast<std::size_t>(indices[i]),
                   base_vertex_index + static_cast<std::size_t>(indices[i + 1]),
                   base_vertex_index +
                       static_cast<std::size_t>(indices[i + 2])});
            }
          }
        }
      } else {
        // 如果没有索引数据，使用顺序索引
        for (std::size_t i = 0; i < positions_accessor.count; i += 3) {
          if (i + 2 < positions_accessor.count) {
            face_indices.push_back({base_vertex_index + i,
                                    base_vertex_index + i + 1,
                                    base_vertex_index + i + 2});
          }
        }
      }
    }
  }

  if (is_glb)
    std::cout << "GLB file loaded successfully: " << filename << std::endl;
  else
    std::cout << "GLTF file loaded successfully: " << filename << std::endl;
  std::cout << "Vertices: " << vertices.size() << std::endl;
  std::cout << "Faces: " << face_indices.size() << std::endl;

  return true;
}