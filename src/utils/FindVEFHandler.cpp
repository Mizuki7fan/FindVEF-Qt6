#include "FindVEFHandler.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

FindVEFHandler::FindVEFHandler() {};

void FindVEFHandler::Load(const fs::path &file_path) {
  Clear();
  path = file_path;
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    std::cerr << "Error: cannot open file " << path << std::endl;
    return;
  }

  // 使用unordered_map替代switch语句，更清晰
  std::unordered_map<std::string, std::function<void(const std::string &)>>
      handlers = {
          {"P", [this](const std::string &line) { ReadPoint(line); }},
          {"V", [this](const std::string &line) { ReadVertex(line); }},
          {"F", [this](const std::string &line) { ReadFace(line); }},
          {"VE", [this](const std::string &line) { ReadVertexEdge(line); }},
          {"PE", [this](const std::string &line) { ReadPointEdge(line); }}};

  std::string line;
  std::function<void(const std::string &)> current_handler = nullptr;

  while (std::getline(ifs, line)) {
    if (IsCommentOrEmpty(line))
      continue;

    // 检查是否是新的段落标识符
    auto it = handlers.find(line);
    if (it != handlers.end()) {
      current_handler = it->second;
      continue;
    }

    // 处理数据行
    if (current_handler) {
      try {
        current_handler(line);
      } catch (const std::exception &e) {
        std::cerr << "Error parsing line: " << line << " - " << e.what()
                  << std::endl;
      }
    }
  }
}

void FindVEFHandler::Clear() {
  path.clear();
  point.clear();
  vertex.clear();
  face.clear();
  point_edge.clear();
  vertex_edge.clear();
}

// 辅助函数：解析颜色
FindVEFHandler::Color3i
FindVEFHandler::ParseColor(std::istringstream &ss,
                           const Color3i &default_color) {
  double r, g, b;
  if (ss >> r >> g >> b) {
    return Color3i{{static_cast<uint8_t>(std::clamp(r, 0.0, 255.0)),
                    static_cast<uint8_t>(std::clamp(g, 0.0, 255.0)),
                    static_cast<uint8_t>(std::clamp(b, 0.0, 255.0))}};
  }
  return default_color;
}

// 辅助函数：检查是否为注释或空行
bool FindVEFHandler::IsCommentOrEmpty(const std::string &line) {
  return line.empty() || line[0] == '#' || line[0] == '/';
}

void FindVEFHandler::ReadVertexEdge(const std::string &line) {
  std::istringstream ss(line);
  unsigned int v0, v1;

  if (!(ss >> v0 >> v1))
    throw std::runtime_error("Failed to read vertex edge indices");

  Color3i color = ParseColor(ss, Color3i{{0, 0, 255}}); // 默认蓝色
  vertex_edge.emplace_back(std::make_pair(v0, v1), color);
}

void FindVEFHandler::ReadVertex(const std::string &line) {
  std::istringstream ss(line);
  unsigned int v;

  if (!(ss >> v))
    throw std::runtime_error("Failed to read vertex index");

  Color3i color = ParseColor(ss, Color3i{{0, 0, 255}}); // 默认蓝色
  vertex.emplace_back(v, color);
}

void FindVEFHandler::ReadFace(const std::string &line) {
  std::istringstream ss(line);
  unsigned int f;

  if (!(ss >> f))
    throw std::runtime_error("Failed to read face index");

  Color3i color = ParseColor(ss, Color3i{{255, 255, 0}}); // 默认黄色
  face.emplace_back(f, color);
}

void FindVEFHandler::ReadPointEdge(const std::string &line) {
  std::istringstream ss(line);
  double x0, y0, z0, x1, y1, z1;

  if (!(ss >> x0 >> y0 >> z0 >> x1 >> y1 >> z1))
    throw std::runtime_error("Failed to read point edge coordinates");

  Color3i color = ParseColor(ss, Color3i{{0, 0, 255}}); // 默认蓝色
  point_edge.emplace_back(
      std::make_pair(Pos3d{{x0, y0, z0}}, Pos3d{{x1, y1, z1}}), color);
}

void FindVEFHandler::ReadPoint(const std::string &line) {
  std::istringstream ss(line);
  double x, y, z;

  if (!(ss >> x >> y >> z))
    throw std::runtime_error("Failed to read point coordinates");

  Color3i color = ParseColor(ss, Color3i{{0, 135, 76}}); // 默认绿色
  point.emplace_back(Pos3d{{x, y, z}}, color);
}