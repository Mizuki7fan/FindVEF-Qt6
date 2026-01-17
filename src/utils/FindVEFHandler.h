#pragma once
#include <array>
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
namespace fs = std::filesystem;
class FindVEFHandler {
public:
  FindVEFHandler();
  void Load(const fs::path &path);
  void Clear();

public:
  bool IsEmpty() const {
    return path.empty() || !std::filesystem::exists(path);
  }
  std::string GetFileName() const { return path.filename().string(); }
  fs::path path;

  // findvef相关，默认含有绘制的颜色
  using Color3i = std::array<uint8_t, 3>;
  using Pos3d = std::array<double, 3>;
  std::vector<std::pair<unsigned int, Color3i>> vertex;
  std::vector<std::pair<unsigned int, Color3i>> face;
  std::vector<std::pair<Pos3d, Color3i>> point;
  std::vector<std::pair<std::pair<Pos3d, Pos3d>, Color3i>> point_edge;
  std::vector<std::pair<std::pair<unsigned int, unsigned int>, Color3i>>
      vertex_edge;

private:
  // 辅助函数
  Color3i ParseColor(std::istringstream &ss, const Color3i &default_color);
  bool IsCommentOrEmpty(const std::string &line);

  // 读取函数
  void ReadVertex(const std::string &line);
  void ReadFace(const std::string &line);
  void ReadVertexEdge(const std::string &line);
  void ReadPointEdge(const std::string &line);
  void ReadPoint(const std::string &line);
};