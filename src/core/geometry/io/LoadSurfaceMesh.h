#pragma once
#include "../surfacemesh.h"
#include <filesystem>
namespace dop::MeshIO {
bool LoadOBJ(const fs::path &filename, SM_with_Meta &mesh_with_meta);
bool SaveOBJ(const fs::path &filename, SM_with_Meta &mesh_with_meta,
             bool triangulate = false);
bool LoadOFF(const fs::path &filename, SM_with_Meta &mesh_with_meta);
bool SaveOFF(const fs::path &filename, SM_with_Meta &mesh_with_meta,
             bool triangulate = false);
bool LoadVTK(const fs::path &filename, SM_with_Meta &mesh_with_meta);
bool LoadPLY(const fs::path &filename, SM_with_Meta &mesh_with_meta);
bool LoadGLTF(const fs::path &filename, SM_with_Meta &mesh_with_meta);
#ifdef SUPPORT_FBX
bool LoadFbx(const fs::path &filename, SM_with_Meta &mesh_with_meta);
#endif
} // namespace dop::MeshIO