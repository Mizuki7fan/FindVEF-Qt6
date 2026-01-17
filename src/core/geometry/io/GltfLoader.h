#pragma once
#include "../SurfaceMesh.h"
#include "tiny_gltf.h"
#include <filesystem>

namespace fs = std::filesystem;

bool LoadGltfFile(const fs::path &filename,             //
                  bool is_glb,                          //
                  std::vector<dop::Point3II> &vertices, //
                  std::vector<std::vector<std::size_t>> &face_indices);