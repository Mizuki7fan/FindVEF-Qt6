#pragma once

#include <fbxsdk.h>

#include <filesystem>

#include "../SurfaceMesh.h"
#include "fbxsdk/scene/geometry/fbxmesh.h"

namespace fs = std::filesystem;

void LoadFbxFile(const fs::path &filename, dop::SM_with_Meta &mesh_with_meta);
void SaveFbxMeshAsObj(const fs::path &filename, fbxsdk::FbxMesh *mesh);