#include "metadata.h"
#include <format>
#include <fstream>
#include <iostream>

Metadata::Metadata() { Clear(); }

void Metadata::Init(fs::path path) {
  Clear();

  this->filepath = path;
  filename = path.stem().string();
  std::string ext = path.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  if (ext == ".obj") {
    format = OBJ;
  } else if (ext == ".off") {
    format = OFF;
  } else if (ext == ".vtk") {
    format = VTK;
  } else if (ext == ".ply") {
    format = PLY;
  } else if (ext == ".fbx") {
    format = DetectFbxFormat(path);
  } else if (ext == ".glb") {
    format = GLB;
  } else if (ext == ".gltf") {
    format = GLTF;
  }

  filesize = fs::file_size(filepath);
}

void Metadata::Clear() {
  filename.clear();
  filepath.clear();
  format = None;
  filesize = 0;
  normal_mapping_mode = NONE;
  texture_mapping_mode = NONE;
  non_manifold_faces.clear();
  num_borders = {0, 0};
  num_cpnts = 0;
}

Metadata::Format Metadata::DetectFbxFormat(const fs::path &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    return FbxAsBinary; // 默认返回binary格式
  }

  // 读取文件头的前23个字节
  char header[24] = {0};
  file.read(header, 23);
  file.close();

  // FBX binary文件以"Kaydara FBX Binary  \x00"开头
  std::string headerStr(header, 23);
  if (headerStr.find("Kaydara FBX Binary") == 0) {
    return FbxAsBinary;
  }

  // FBX text文件通常以"; FBX"或直接以"FBX"开头
  std::string headerStr_short(header, 10);
  if (headerStr_short.find("; FBX") != std::string::npos ||
      headerStr_short.find("FBX") == 0) {
    return FbxAsText;
  }

  // 默认返回binary格式
  return FbxAsBinary;
}

void Metadata::PrintInfo() {
  std::cout << std::format("File: {}", filepath.string()) << std::endl;
  std::cout << std::format("Size: {} bytes", filesize) << std::endl;
  std::cout << std::format("Format: {}", GetFormatString()) << std::endl;
  std::cout << std::format("Normal mapping mode: {}",
                           GetNormalMappingModeString())
            << std::endl;
  std::cout << std::format("Texture mapping mode: {}",
                           GetTextureMappingModeString())
            << std::endl;
}

std::string Metadata::GetFormatString() const {
  switch (format) {
  case OBJ:
    return "OBJ";
  case OFF:
    return "OFF";
  case VTK:
    return "VTK";
  case PLY:
    return "PLY";
  case FbxAsBinary:
    return "FBX (binary)";
  case FbxAsText:
    return "FBX (text)";
  case GLB:
    return "GLB";
  case GLTF:
    return "GLTF";
  default:
    return "Unknown";
  }
}

std::string Metadata::GetNormalMappingModeString() const {
  switch (normal_mapping_mode) {
  case NONE:
    return "NONE";
  case ByVertex:
    return "ByVertex";
  case ByFace:
    return "ByFace";
  case ByHalfedge:
    return "ByHalfedge";
  default:
    return "Unknown";
  }
}

std::string Metadata::GetTextureMappingModeString() const {
  switch (texture_mapping_mode) {
  case NONE:
    return "NONE";
  case ByVertex:
    return "ByVertex";
  case ByFace:
    return "ByFace, Error";
  case ByHalfedge:
    return "ByHalfedge";
  default:
    return "Unknown";
  }
}
