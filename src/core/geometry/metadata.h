#pragma once
#include <array>
#include <cstddef>
#include <filesystem>
#include <format>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// 模型元信息数据结构
class Metadata {
public:
  // 枚举定义需要在使用之前
  enum Format {
    None = 0,
    OBJ = 1,
    OFF = 2,
    VTK = 3,
    PLY = 4,
    FbxAsBinary = 5,
    FbxAsText = 6,
    GLB = 7,
    GLTF = 8
  };
  // 元素映射模式
  enum ElementMappingMode {
    NONE = 0,
    ByVertex = 1,
    ByFace = 2,
    ByHalfedge = 3
  };

  Metadata(); // 默认构造函数
  void Init(fs::path filepath);
  void Clear();
  void PrintInfo();
  std::string GetFileName() const { return filename; }
  std::string GetFileNameWithExt() const {
    return std::format("{}.{}", filename, GetFormatString());
  }
  fs::path GetFilePath() const { return filepath; }
  std::size_t GetFileSize() const { return filesize; }
  Format GetFormat() const { return format; }
  std::string GetFormatString() const;
  std::string GetNormalMappingModeString() const;
  std::string GetTextureMappingModeString() const;

  void SetNormalMappingMode(ElementMappingMode mode) {
    normal_mapping_mode = mode;
  }
  ElementMappingMode GetNormalMappingMode() const {
    return normal_mapping_mode;
  }

  void SetTextureMappingMode(ElementMappingMode mode) {
    texture_mapping_mode = mode;
  }
  ElementMappingMode GetTextureMappingMode() const {
    return texture_mapping_mode;
  }

  void AddNonManifoldFace(const std::vector<std::array<double, 3>> &face) {
    non_manifold_faces.push_back(face);
  }
  const std::vector<std::vector<std::array<double, 3>>> &
  GetNonManifoldFaces() const {
    return non_manifold_faces;
  }
  void SetNumCPnts(std::size_t num) { num_cpnts = num; }
  std::size_t GetNumCPnts() const { return num_cpnts; }
  void SetNumBorders(std::array<std::size_t, 2> borders) {
    num_borders = borders;
  }
  std::array<std::size_t, 2> GetNumBorders() const { return num_borders; }

private:
  Format DetectFbxFormat(const fs::path &path);

  fs::path filepath;    // 完整文件路径
  std::string filename; // 文件名（不含路径）
  Format format;        // 文件格式 (obj, off, vtk, fbx等)
  std::size_t filesize; // 文件大小（字节）

  ElementMappingMode normal_mapping_mode = NONE;
  ElementMappingMode texture_mapping_mode = NONE;
  std::vector<std::vector<std::array<double, 3>>> non_manifold_faces;
  std::size_t num_cpnts = 0; // 0表示没有计算
  std::array<std::size_t, 2> num_borders = {0, 0};
};
