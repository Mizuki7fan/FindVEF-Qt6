#include "FbxLoader.h"
#include "../../../utils/FVAssert.h"
#include "../metadata.h"
#include "../operation.h"
#include "../surfacemesh.h"
#include <CGAL/IO/OBJ.h>
#include <algorithm>

bool WriteSurfaceMeshWithNormalAsObj(std::ofstream &out,
                                     const dop::SurfaceMesh3 &mesh) {
  // 调试用
  if (!out.is_open()) {
    std::cerr << "Output file stream is not open" << std::endl;
    return false;
  }

  std::optional<
      dop::SurfaceMesh3::Property_map<CGAL::SM_Halfedge_index, dop::Vec3II>>
      halfedge_normals =
          mesh.property_map<CGAL::SM_Halfedge_index, dop::Vec3II>("h:normal");
  std::optional<
      dop::SurfaceMesh3::Property_map<CGAL::SM_Vertex_index, dop::Vec3II>>
      vertex_normals =
          mesh.property_map<CGAL::SM_Vertex_index, dop::Vec3II>("v:normal");
  std::optional<
      dop::SurfaceMesh3::Property_map<CGAL::SM_Face_index, dop::Vec3II>>
      face_normals =
          mesh.property_map<CGAL::SM_Face_index, dop::Vec3II>("f:normal");
  std::optional<
      dop::SurfaceMesh3::Property_map<CGAL::SM_Vertex_index, dop::Point2II>>
      vertex_uv =
          mesh.property_map<CGAL::SM_Vertex_index, dop::Point2II>("v:uv");
  std::optional<
      dop::SurfaceMesh3::Property_map<CGAL::SM_Halfedge_index, dop::Point2II>>
      halfedge_uv =
          mesh.property_map<CGAL::SM_Halfedge_index, dop::Point2II>("h:uv");

  const bool use_halfedge_uv = halfedge_uv.has_value();
  const bool use_vertex_uv =
      vertex_uv.has_value() && !use_halfedge_uv; // 若有h:uv则优先用h:uv

  std::vector<dop::Point3II> vertices;
  std::vector<std::vector<CGAL::SM_Vertex_index>> face_vertices;
  std::vector<dop::Vec3II> normal_array;
  std::vector<dop::Point2II> texture_array;
  std::vector<std::size_t>
      face_vt_offsets; // 记录每个面的vt起始偏移（仅h:uv时使用）

  // 输出顶点坐标与按顶点的纹理与法线收集
  for (CGAL::SM_Vertex_index vertex : mesh.vertices()) {
    if (vertex_normals.has_value())
      normal_array.push_back(vertex_normals.value()[vertex]);
    if (use_vertex_uv)
      texture_array.push_back(vertex_uv.value()[vertex]);
    vertices.push_back(mesh.point(vertex));
  }

  // 当使用半边属性时，在遍历面与半边时按顺序收集vt与半边法线
  for (CGAL::SM_Face_index face : mesh.faces()) {
    face_vertices.push_back(std::vector<CGAL::SM_Vertex_index>());
    auto &face_vertex = face_vertices.back();

    if (face_normals.has_value())
      normal_array.push_back(face_normals.value()[face]);

    if (use_halfedge_uv)
      face_vt_offsets.push_back(texture_array.size());

    for (CGAL::SM_Halfedge_index halfedge :
         mesh.halfedges_around_face(mesh.halfedge(face))) {
      face_vertex.push_back(mesh.source(halfedge));
      if (halfedge_normals.has_value())
        normal_array.push_back(halfedge_normals.value()[halfedge]);
      if (use_halfedge_uv)
        texture_array.push_back(halfedge_uv.value()[halfedge]);
    }
  }

  for (const auto &vertex : vertices) {
    out << "v " << vertex.x() << " " << vertex.y() << " " << vertex.z()
        << std::endl;
  }

  for (const auto &texture : texture_array) {
    out << "vt " << texture.x() << " " << texture.y() << std::endl;
  }

  for (const auto &normal : normal_array) {
    out << "vn " << normal.x() << " " << normal.y() << " " << normal.z()
        << std::endl;
  }

  std::size_t face_idx = 0;
  std::size_t accu_normal_idx = 0;
  for (const auto &face_vertex : face_vertices) {
    out << "f";
    for (std::size_t i = 0; i < face_vertex.size(); ++i) {
      const auto &vertex = face_vertex[i];
      const int v_idx = static_cast<int>(vertex.idx()) + 1;
      const bool has_vt = use_vertex_uv || use_halfedge_uv;
      const bool has_vn_any = vertex_normals.has_value() ||
                              face_normals.has_value() ||
                              halfedge_normals.has_value();

      int vt_idx = -1;
      if (use_vertex_uv) {
        // 顶点UV：与顶点索引一一对应
        vt_idx = v_idx; // 1-based index
      } else if (use_halfedge_uv) {
        // 半边UV：按我们收集texture_array的顺序（face_vt_offsets + i）
        if (face_idx < face_vt_offsets.size()) {
          vt_idx = static_cast<int>(face_vt_offsets[face_idx] + i + 1);
        }
      }

      int vn_idx = -1;
      if (vertex_normals.has_value()) {
        vn_idx = v_idx; // 顶点法线：与顶点索引一致
      } else if (face_normals.has_value()) {
        vn_idx = static_cast<int>(face_idx) + 1; // 面法线：使用面索引
      } else if (halfedge_normals.has_value()) {
        // 半边法线：按照收集顺序为每个面内顶点递增
        vn_idx = static_cast<int>(accu_normal_idx + i + 1);
      }

      // 写入v/(vt)/(vn)
      out << " " << v_idx;
      if (has_vt && has_vn_any && vn_idx > 0) {
        // v/vt/vn 三者都有
        out << "/" << vt_idx << "/" << vn_idx;
      } else if (has_vt && (!has_vn_any || vn_idx <= 0)) {
        // 只有纹理：v/vt
        out << "/" << vt_idx;
      } else if (!has_vt && has_vn_any && vn_idx > 0) {
        // 只有法线：v//vn（注意双斜杠）
        out << "//" << vn_idx;
      } // 否则仅输出 v
    }
    // 面结束时处理半边法线的累计偏移，并换行与推进面索引
    if (halfedge_normals.has_value()) {
      accu_normal_idx += face_vertex.size();
    }
    out << std::endl;
    ++face_idx;
  }

  return true;
}

bool WriteSurfaceMeshTextureAsObj(std::ofstream &out,
                                  const dop::SurfaceMesh3 &mesh) {
  if (!out.is_open()) {
    std::cerr << "Output file stream is not open" << std::endl;
    return false;
  }

  std::optional<
      dop::SurfaceMesh3::Property_map<CGAL::SM_Vertex_index, dop::Point2II>>
      vertex_uv =
          mesh.property_map<CGAL::SM_Vertex_index, dop::Point2II>("v:uv");
  std::optional<
      dop::SurfaceMesh3::Property_map<CGAL::SM_Halfedge_index, dop::Point2II>>
      halfedge_uv =
          mesh.property_map<CGAL::SM_Halfedge_index, dop::Point2II>("h:uv");

  if (!vertex_uv.has_value() && !halfedge_uv.has_value())
    return false;

  std::vector<dop::Point3II> vertices;
  std::vector<std::vector<std::size_t>> face_vertices;

  if (vertex_uv.has_value() && !halfedge_uv.has_value()) {
    // 按顶点UV构建UV网格（每个顶点一个坐标）
    for (CGAL::SM_Vertex_index vertex : mesh.vertices()) {
      dop::Point3II p(vertex_uv.value()[vertex].x(),
                      vertex_uv.value()[vertex].y(), 0);
      vertices.push_back(p);
    }

    for (CGAL::SM_Face_index face : mesh.faces()) {
      face_vertices.push_back(std::vector<std::size_t>());
      auto &face_vertex = face_vertices.back();
      for (CGAL::SM_Halfedge_index halfedge :
           mesh.halfedges_around_face(mesh.halfedge(face)))
        face_vertex.push_back(mesh.source(halfedge).idx());
    }
  } else {
    // 按半边UV构建UV网格（每个面-顶点一套坐标，避免共享）
    std::size_t base = 0;
    for (CGAL::SM_Face_index face : mesh.faces()) {
      std::vector<std::size_t> fv;
      for (CGAL::SM_Halfedge_index he :
           mesh.halfedges_around_face(mesh.halfedge(face))) {
        const auto &uv = halfedge_uv.value()[he];
        vertices.emplace_back(uv.x(), uv.y(), 0);
        fv.push_back(base++);
      }
      if (!fv.empty())
        face_vertices.push_back(std::move(fv));
    }
  }

  for (const auto &vertex : vertices) {
    out << "v " << vertex.x() << " " << vertex.y() << " " << vertex.z()
        << std::endl;
  }

  dop::SurfaceMesh3 uv_mesh;
  dop::MeshOperation::BuildSurfaceMesh(vertices, face_vertices, uv_mesh);
  CGAL::IO::write_OBJ(out, uv_mesh);

  return true;
}

// 将法线映射模式设置与法线数组读取封装为函数
static void SetupNormalMappingAndArray(fbxsdk::FbxMesh *fbx_mesh,
                                       Metadata &meta,
                                       std::vector<dop::Vec3II> &normal_array) {
  if (!fbx_mesh)
    return;

  fbxsdk::FbxLayerElementNormal *normal_element = fbx_mesh->GetElementNormal();

  if (normal_element == nullptr)
    meta.SetNormalMappingMode(Metadata::NONE);
  else if (normal_element->GetMappingMode() ==
           fbxsdk::FbxLayerElement::eByControlPoint)
    meta.SetNormalMappingMode(Metadata::ByVertex);
  else if (normal_element->GetMappingMode() ==
           fbxsdk::FbxLayerElement::eByPolygon)
    meta.SetNormalMappingMode(Metadata::ByFace);
  else if (normal_element->GetMappingMode() ==
           fbxsdk::FbxLayerElement::eByPolygonVertex)
    meta.SetNormalMappingMode(Metadata::ByHalfedge);
  else
    meta.SetNormalMappingMode(Metadata::NONE);

  normal_array.clear();
  if (meta.GetNormalMappingMode() == Metadata::NONE)
    return;
  const fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector4> &normals =
      normal_element->GetDirectArray();
  normal_array.reserve(normals.GetCount());
  for (int i = 0; i < normals.GetCount(); ++i) {
    const fbxsdk::FbxVector4 &n = normals[i];
    normal_array.emplace_back(n[0], n[1], n[2]);
  }
}

// 读取并聚合UV为按顶点属性，返回是否存在任何UV
static void
SetupTextureMappingAndArray(fbxsdk::FbxMesh *fbx_mesh, //
                            Metadata &meta,            //
                            std::vector<dop::Point2II> &texture_array) {
  if (!fbx_mesh)
    return;

  // 默认只取第一套纹理
  const int uv_layer_count = fbx_mesh->GetElementUVCount();
  fbxsdk::FbxLayerElementUV *texture_element =
      uv_layer_count > 0 ? fbx_mesh->GetElementUV(0) : nullptr;

  if (texture_element == nullptr) {
    meta.SetTextureMappingMode(Metadata::NONE);
  } else if (texture_element->GetMappingMode() ==
             fbxsdk::FbxLayerElement::eByControlPoint) {
    meta.SetTextureMappingMode(Metadata::ByVertex);
  } else if (texture_element->GetMappingMode() ==
             fbxsdk::FbxLayerElement::eByPolygonVertex) {
    meta.SetTextureMappingMode(Metadata::ByHalfedge);
  } else {
    meta.SetTextureMappingMode(Metadata::NONE);
  }

  texture_array.clear();
  if (meta.GetTextureMappingMode() == Metadata::NONE ||
      texture_element == nullptr)
    return;

  // 直接数组填充（不做聚合）。IndexToDirect 时通过 IndexArray
  // 在后续赋值阶段映射。
  const auto &direct = texture_element->GetDirectArray();
  texture_array.reserve(direct.GetCount());
  for (int i = 0; i < direct.GetCount(); ++i) {
    const fbxsdk::FbxVector2 &uv = direct.GetAt(i);
    texture_array.emplace_back(uv[0], uv[1]);
  }
}

bool TraverseFbxNode(FbxNode *node,
                     const std::function<bool(FbxNode *)> &func) {
  if (!func(node))
    return false;

  for (auto child_num = node->GetChildCount(), i = 0; i < child_num; ++i)
    if (!TraverseFbxNode(node->GetChild(i), func))
      return false;
  return true;
}

std::vector<FbxNode *> GetSubordinateFbxNodes(FbxNode *root_node) {
  std::vector<FbxNode *> nodes;
  TraverseFbxNode(root_node, [&](FbxNode *node) {
    nodes.push_back(node);
    return true;
  });
  return nodes;
}

fbxsdk::FbxScene *ApplyFbxSceneTransform(fbxsdk::FbxScene *scene) {
  if (!scene || !scene->GetRootNode()) {
    std::cerr << "Invalid FBX scene or root node" << std::endl;
    return nullptr;
  }

  std::vector<fbxsdk::FbxNode *> fbx_nodes =
      GetSubordinateFbxNodes(scene->GetRootNode()); // 取当前scene的每个node
  // 移除所有不包含Mesh的node
  fbx_nodes.erase(std::remove_if(fbx_nodes.begin(), fbx_nodes.end(),
                                 [&](auto node) -> bool {
                                   return !node || !node->GetMesh();
                                 }),
                  fbx_nodes.end());

  // 如果没有有效的网格节点，返回原场景
  if (fbx_nodes.empty()) {
    std::cerr << "No valid mesh nodes found in the scene" << std::endl;
    return scene;
  }

  // 变换和node绑定, 逐node处理
  for (const auto &fbx_node : fbx_nodes) {
    // 取全局变换矩阵
    FbxAMatrix global_transform =
        fbx_node->EvaluateGlobalTransform(FBXSDK_TIME_INFINITE);
    FbxAMatrix normal_transform = global_transform;
    normal_transform = normal_transform.Inverse();
    normal_transform = normal_transform.Transpose();

    FbxMesh *mesh = fbx_node->GetMesh();
    if (!mesh)
      continue; // 安全检查

    // 取mesh, 根据变换修改顶点
    int control_points_count = mesh->GetControlPointsCount();
    for (int i = 0; i < control_points_count; i++) {
      FbxVector4 point = mesh->GetControlPointAt(i);
      point = global_transform.MultT(point);
      mesh->SetControlPointAt(point, i);
    }

    // 取法线, 根据变换修改法线
    if (mesh->GetElementNormalCount() > 0) {
      const auto &normal_element = mesh->GetElementNormal(0);
      if (normal_element != nullptr) {
        int normal_count = normal_element->GetDirectArray().GetCount();
        for (int i = 0; i < normal_count; i++) {
          FbxVector4 normal_point = normal_element->GetDirectArray().GetAt(i);
          normal_point = normal_transform.MultT(normal_point);
          normal_point.Normalize();
          // 直接修改原始法线数组
          const_cast<FbxLayerElementArrayTemplate<FbxVector4> &>(
              normal_element->GetDirectArray())
              .SetAt(i, normal_point);
        }
      }
    }

    // 重置节点变换为单位矩阵
    fbx_node->LclTranslation.Set(FbxVector4(0, 0, 0));
    fbx_node->LclRotation.Set(FbxVector4(0, 0, 0));
    fbx_node->LclScaling.Set(FbxVector4(1, 1, 1));
  }

  return scene; // 返回修改后的原场景
}

fbxsdk::FbxScene *MergeFbxNodesToOneNode(fbxsdk::FbxScene *scene) {
  if (!scene || !scene->GetRootNode()) {
    std::cerr << "Invalid FBX scene or root node" << std::endl;
    return nullptr;
  }

  auto fbx_nodes = GetSubordinateFbxNodes(scene->GetRootNode());
  // 移除所有不包含Mesh的node
  fbx_nodes.erase(std::remove_if(fbx_nodes.begin(), fbx_nodes.end(),
                                 [&](auto node) -> bool {
                                   return !node || !node->GetMesh();
                                 }),
                  fbx_nodes.end());

  // 如果没有有效的网格节点，返回原场景
  if (fbx_nodes.empty()) {
    std::cerr << "No valid mesh nodes found in the scene" << std::endl;
    return scene;
  }

  // creat scene
  fbxsdk::FbxManager *manager = fbxsdk::FbxManager::Create();
  if (!manager) {
    std::cerr << "Failed to create FBX manager" << std::endl;
    return scene; // 返回原场景
  }

  FbxScene *new_scene = FbxScene::Create(manager, "scene");
  if (!new_scene) {
    std::cerr << "Failed to create new FBX scene" << std::endl;
    manager->Destroy();
    return scene; // 返回原场景
  }

  FbxNode *new_root = new_scene->GetRootNode();
  if (!new_root) {
    std::cerr << "Failed to get root node of new scene" << std::endl;
    new_scene->Destroy();
    return scene; // 返回原场景
  }

  // creat new node
  FbxNode *new_node = FbxNode::Create(new_scene, "root node");
  if (!new_node) {
    std::cerr << "Failed to create new node" << std::endl;
    new_scene->Destroy();
    return scene; // 返回原场景
  }

  FbxMesh *merged_mesh = FbxMesh::Create(new_scene, "root mesh");
  if (!merged_mesh) {
    std::cerr << "Failed to create merged mesh" << std::endl;
    new_scene->Destroy();
    return scene; // 返回原场景
  }

  // add vertex
  std::vector<FbxVector4> global_control_points;
  std::vector<FbxVector4> global_normal_array;

  std::vector<std::map<int, int>>
      map_node_idx_to_map_origin_vtx_idx_to_global_vtx_idx;
  std::vector<std::map<int, int>>
      map_node_idx_to_map_origin_normal_idx_to_global_normal_idx;

  std::map<int, int> map_global_plg_vtx_idx_to_global_normal_index;

  int global_plg_vtx_idx = 0;

  try {
    // 遍历每个node
    for (const auto &fbx_node : fbx_nodes) {
      fbxsdk::FbxMesh *fbx_mesh = fbx_node->GetMesh();
      if (!fbx_mesh)
        continue; // 安全检查

      int plgs_num = fbx_mesh->GetPolygonCount();

      const auto &normal_element = fbx_mesh->GetElementNormalCount() > 0
                                       ? fbx_mesh->GetElementNormal(0)
                                       : nullptr;
      bool has_normals = (normal_element != nullptr);

      std::map<int, int> map_origin_vtx_idx_to_global_vtx_idx;
      std::map<int, int> map_origin_normal_idx_to_global_normal_idx;

      for (int plg_idx = 0; plg_idx < plgs_num; ++plg_idx) {
        const auto vtx_num = fbx_mesh->GetPolygonSize(plg_idx);
        for (int vtx_pos = 0; vtx_pos < vtx_num; vtx_pos++) {
          int origin_vtx_idx = fbx_mesh->GetPolygonVertex(plg_idx, vtx_pos);
          if (origin_vtx_idx < 0 ||
              origin_vtx_idx >= fbx_mesh->GetControlPointsCount()) {
            std::cerr << "Invalid vertex index: " << origin_vtx_idx
                      << std::endl;
            continue;
          }

          if (!map_origin_vtx_idx_to_global_vtx_idx.contains(origin_vtx_idx)) {
            auto point = fbx_mesh->GetControlPointAt(origin_vtx_idx);
            map_origin_vtx_idx_to_global_vtx_idx[origin_vtx_idx] =
                static_cast<int>(global_control_points.size());
            global_control_points.push_back(point);
          }

          if (has_normals) {
            int origin_normal_idx = -1;
            int polygon_vertex_idx =
                fbx_mesh->GetPolygonVertexIndex(plg_idx) + vtx_pos;

            if (normal_element->GetMappingMode() ==
                FbxLayerElement::eByPolygonVertex) {
              if (normal_element->GetReferenceMode() ==
                  FbxLayerElement::eDirect) {
                origin_normal_idx = polygon_vertex_idx;
              } else if (normal_element->GetReferenceMode() ==
                         FbxLayerElement::eIndexToDirect) {
                if (polygon_vertex_idx >= 0 &&
                    polygon_vertex_idx <
                        normal_element->GetIndexArray().GetCount()) {
                  origin_normal_idx =
                      normal_element->GetIndexArray().GetAt(polygon_vertex_idx);
                }
              }
            } else if (normal_element->GetMappingMode() ==
                       FbxLayerElement::eByControlPoint) {
              if (normal_element->GetReferenceMode() ==
                  FbxLayerElement::eDirect) {
                origin_normal_idx = origin_vtx_idx;
              } else if (normal_element->GetReferenceMode() ==
                         FbxLayerElement::eIndexToDirect) {
                if (origin_vtx_idx >= 0 &&
                    origin_vtx_idx <
                        normal_element->GetIndexArray().GetCount()) {
                  origin_normal_idx =
                      normal_element->GetIndexArray().GetAt(origin_vtx_idx);
                }
              }
            }

            if (origin_normal_idx >= 0 &&
                origin_normal_idx <
                    normal_element->GetDirectArray().GetCount()) {
              auto normal_point =
                  normal_element->GetDirectArray().GetAt(origin_normal_idx);
              int global_normal_index =
                  static_cast<int>(global_normal_array.size());
              global_normal_array.push_back(normal_point);
              map_global_plg_vtx_idx_to_global_normal_index
                  [global_plg_vtx_idx] = global_normal_index;
            }
          }

          global_plg_vtx_idx++;
        }
      }

      map_node_idx_to_map_origin_vtx_idx_to_global_vtx_idx.push_back(
          std::move(map_origin_vtx_idx_to_global_vtx_idx));
      map_node_idx_to_map_origin_normal_idx_to_global_normal_idx.push_back(
          std::move(map_origin_normal_idx_to_global_normal_idx));
    }

    // 如果没有收集到顶点，返回原场景
    if (global_control_points.empty()) {
      std::cerr << "No vertices collected from meshes" << std::endl;
      new_scene->Destroy();
      return scene;
    }

    std::size_t global_vertices_num = global_control_points.size();
    merged_mesh->InitControlPoints(static_cast<int>(global_vertices_num));
    for (std::size_t global_vtx_idx = 0; global_vtx_idx < global_vertices_num;
         ++global_vtx_idx) {
      merged_mesh->SetControlPointAt(global_control_points[global_vtx_idx],
                                     static_cast<int>(global_vtx_idx));
    }

    // add polygons to mesh
    std::vector<FbxVector2> global_uv_array;
    std::map<int, int> map_global_plg_vtx_idx_to_global_uv_index;
    global_plg_vtx_idx = 0;
    for (std::size_t node_idx = 0; node_idx < fbx_nodes.size(); ++node_idx) {
      if (node_idx >=
          map_node_idx_to_map_origin_vtx_idx_to_global_vtx_idx.size()) {
        continue; // 安全检查
      }

      const auto &map_origin_vtx_idx_to_global_vtx_idx =
          map_node_idx_to_map_origin_vtx_idx_to_global_vtx_idx[node_idx];
      const auto &fbx_node = fbx_nodes[node_idx];
      fbxsdk::FbxMesh *fbx_mesh = fbx_node->GetMesh();
      if (!fbx_mesh)
        continue; // 安全检查

      const auto &element_uv = fbx_mesh->GetElementUVCount() > 0
                                   ? fbx_mesh->GetElementUV(0)
                                   : nullptr;
      bool has_uv = element_uv != nullptr;
      std::map<int, int> map_origin_uv_index_to_global_uv_index;

      int plgs_num = fbx_mesh->GetPolygonCount();
      for (int plg_idx = 0; plg_idx < plgs_num; ++plg_idx) {
        merged_mesh->BeginPolygon(-1, -1, -1, false);
        const auto vtx_num = fbx_mesh->GetPolygonSize(plg_idx);
        for (int vtx_pos = 0; vtx_pos < vtx_num; vtx_pos++) {
          int origin_fbx_vtx = fbx_mesh->GetPolygonVertex(plg_idx, vtx_pos);
          if (origin_fbx_vtx < 0 ||
              !map_origin_vtx_idx_to_global_vtx_idx.contains(origin_fbx_vtx)) {
            continue; // 安全检查
          }

          int global_vtx_idx =
              map_origin_vtx_idx_to_global_vtx_idx.at(origin_fbx_vtx);
          merged_mesh->AddPolygon(global_vtx_idx);

          if (has_uv) {
            try {
              const auto origin_uv_idx =
                  fbx_mesh->GetTextureUVIndex(plg_idx, vtx_pos);
              if (origin_uv_idx >= 0 &&
                  origin_uv_idx < element_uv->GetDirectArray().GetCount()) {
                if (!map_origin_uv_index_to_global_uv_index.contains(
                        origin_uv_idx)) {
                  const auto uv_coordinate =
                      element_uv->GetDirectArray().GetAt(origin_uv_idx);
                  map_origin_uv_index_to_global_uv_index[origin_uv_idx] =
                      static_cast<int>(global_uv_array.size());
                  global_uv_array.push_back(uv_coordinate);
                }
                int global_uv_index =
                    map_origin_uv_index_to_global_uv_index[origin_uv_idx];
                map_global_plg_vtx_idx_to_global_uv_index[global_plg_vtx_idx] =
                    global_uv_index;
              }
            } catch (const std::exception &e) {
              std::cerr << "Error processing UV: " << e.what() << std::endl;
            }
          }

          global_plg_vtx_idx++;
        }
        merged_mesh->EndPolygon();
      }
    }

    // add uv
    if (!global_uv_array.empty()) {
      // 1. 获取或创建基础图层（Layer 0）
      FbxLayer *layer = merged_mesh->GetLayer(0);
      if (!layer) {
        merged_mesh->CreateLayer();
        layer = merged_mesh->GetLayer(0);
      }

      if (layer) {
        // 2. 创建UV层元素
        FbxLayerElementUV *uv_layer =
            FbxLayerElementUV::Create(merged_mesh, "DiffuseUV");

        // 3. 设置映射模式（按多边形顶点）
        uv_layer->SetMappingMode(FbxLayerElement::eByPolygonVertex);

        // 4. 设置引用模式（索引到直接数组）
        uv_layer->SetReferenceMode(FbxLayerElement::eIndexToDirect);

        // 5. 填充UV坐标数据（直接数组）
        uv_layer->GetDirectArray().SetCount(
            static_cast<int>(global_uv_array.size()));
        for (std::size_t i = 0; i < global_uv_array.size(); i++) {
          uv_layer->GetDirectArray().SetAt(static_cast<int>(i),
                                           global_uv_array[i]);
        }

        // 6. 填充UV索引数据
        uv_layer->GetIndexArray().SetCount(global_plg_vtx_idx);
        for (const auto &[global_plg_vtx_idx, global_uv_index] :
             map_global_plg_vtx_idx_to_global_uv_index) {
          if (global_plg_vtx_idx >= 0 &&
              global_plg_vtx_idx < uv_layer->GetIndexArray().GetCount()) {
            uv_layer->GetIndexArray().SetAt(global_plg_vtx_idx,
                                            global_uv_index);
          }
        }

        // 7. 将UV层添加到网格
        layer->SetUVs(uv_layer, FbxLayerElement::eTextureDiffuse);
      }
    }

    if (!global_normal_array.empty()) {
      FbxLayer *layer = merged_mesh->GetLayer(0);
      if (!layer) {
        merged_mesh->CreateLayer();
        layer = merged_mesh->GetLayer(0);
      }

      if (layer) {
        FbxLayerElementNormal *normal_layer =
            FbxLayerElementNormal::Create(merged_mesh, "Normal");
        normal_layer->SetMappingMode(FbxLayerElement::eByPolygonVertex);
        normal_layer->SetReferenceMode(FbxLayerElement::eIndexToDirect);

        normal_layer->GetDirectArray().SetCount(
            static_cast<int>(global_normal_array.size()));
        for (std::size_t i = 0; i < global_normal_array.size(); i++) {
          normal_layer->GetDirectArray().SetAt(static_cast<int>(i),
                                               global_normal_array[i]);
        }

        normal_layer->GetIndexArray().SetCount(global_plg_vtx_idx);
        for (const auto &entry :
             map_global_plg_vtx_idx_to_global_normal_index) {
          if (entry.first >= 0 &&
              entry.first < normal_layer->GetIndexArray().GetCount()) {
            normal_layer->GetIndexArray().SetAt(entry.first, entry.second);
          }
        }

        layer->SetNormals(normal_layer);
      }
    }

    new_node->SetNodeAttribute(merged_mesh);
    new_root->AddChild(new_node);

    return new_scene;
  } catch (const std::exception &e) {
    std::cerr << "Exception in MergeFbxNodesToOneNode: " << e.what()
              << std::endl;
    if (new_scene) {
      new_scene->Destroy();
    }
    return scene; // 返回原场景
  }
}

fbxsdk::FbxScene *LoadFbxScene(const fs::path &filename) {
  bool status;
  auto manager = FbxManager::Create();
  if (!manager) {
    std::cerr << "Failed to create FBX manager" << std::endl;
    return nullptr;
  }

  // 创建并配置IOSettings
  FbxIOSettings *ios_settings = FbxIOSettings::Create(manager, IOSROOT);
  if (!ios_settings) {
    std::cerr << "Failed to create FBX IO settings" << std::endl;
    manager->Destroy();
    return nullptr;
  }

  // 只读取骨骼动画相关的数据，禁用其他所有数据
  ios_settings->SetBoolProp(IMP_FBX_MATERIAL, true);        // 不需要材质
  ios_settings->SetBoolProp(IMP_FBX_TEXTURE, true);         // 不需要纹理
  ios_settings->SetBoolProp(IMP_FBX_LINK, true);            // 启用骨骼绑定信息
  ios_settings->SetBoolProp(IMP_FBX_SHAPE, true);           // 不需要形状变形
  ios_settings->SetBoolProp(IMP_FBX_GOBO, true);            // 不需要光照投影
  ios_settings->SetBoolProp(IMP_FBX_ANIMATION, true);       // 启用动画数据
  ios_settings->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true); // 保留全局设置

  // 将IOSettings设置为manager的默认设置
  manager->SetIOSettings(ios_settings);

  FbxScene *fbx_scene = FbxScene::Create(manager, filename.string().c_str());
  if (!fbx_scene) {
    std::cerr << "Failed to create FBX scene" << std::endl;
    manager->Destroy();
    return nullptr;
  }

  // Create an importer.
  FbxImporter *importer = FbxImporter::Create(manager, "");
  if (!importer) {
    std::cerr << "Failed to create FBX importer" << std::endl;
    manager->Destroy();
    return nullptr;
  }

  // 初始化, 如果失败则返回nullptr
  if (!importer->Initialize(filename.string().c_str(), -1, ios_settings)) {
    std::cerr << "importer->Initialize failed" << std::endl;
    importer->Destroy();
    manager->Destroy();
    return nullptr;
  }

  status = importer->Import(fbx_scene);
  if (!status && importer->GetStatus().GetCode() == FbxStatus::ePasswordError) {
    std::cerr << "Password error" << std::endl;
    importer->Destroy();
    manager->Destroy();
    return nullptr;
  }

  importer->Destroy();

  // 注意：这里不销毁manager，因为fbx_scene依赖它
  // manager会在fbx_scene被销毁时一起销毁
  return fbx_scene;
}

void CreateSurfaceMesh3FromFbxMesh(fbxsdk::FbxMesh *fbx_mesh,
                                   dop::SM_with_Meta &mesh_with_meta) {
  if (!fbx_mesh)
    return;

  dop::SurfaceMesh3 &mesh = mesh_with_meta.mesh;
  Metadata &meta = mesh_with_meta.metadata;

  std::vector<dop::Point3II> vertices;
  std::vector<std::vector<std::size_t>> face_vertex_indices;  // 顶点索引
  std::vector<std::vector<std::size_t>> face_normal_indices;  // 法向量索引
  std::vector<std::vector<std::size_t>> face_texture_indices; // 纹理坐标索引

  // 取法线法向量列表
  fbxsdk::FbxLayerElementNormal *normal_element = fbx_mesh->GetElementNormal();
  std::vector<dop::Vec3II> normal_array;
  SetupNormalMappingAndArray(fbx_mesh, meta, normal_array);

  // 取纹理坐标列表
  std::vector<dop::Point2II> texture_array;
  SetupTextureMappingAndArray(fbx_mesh, meta, texture_array);

  // 装填顶点信息
  const int num_vertices = fbx_mesh->GetControlPointsCount();
  const fbxsdk::FbxVector4 *control_points = fbx_mesh->GetControlPoints();
  vertices.reserve(num_vertices);
  for (int i = 0; i < num_vertices; ++i)
    vertices.emplace_back(control_points[i][0], control_points[i][1],
                          control_points[i][2]);

  // 装填面信息
  const int num_faces = fbx_mesh->GetPolygonCount();
  face_vertex_indices.reserve(num_faces);
  face_normal_indices.reserve(num_faces);
  face_texture_indices.reserve(num_faces);

  // 装填面/法线/纹理的idx信息
  for (int i = 0; i < num_faces; ++i) {
    face_vertex_indices.push_back(std::vector<std::size_t>());
    face_normal_indices.push_back(std::vector<std::size_t>());
    face_texture_indices.push_back(std::vector<std::size_t>());
    std::vector<std::size_t> &vertex_indices = face_vertex_indices.back();
    std::vector<std::size_t> &normal_indices = face_normal_indices.back();
    std::vector<std::size_t> &texture_indices = face_texture_indices.back();

    const int plg_size = fbx_mesh->GetPolygonSize(i);
    vertex_indices.reserve(plg_size);
    normal_indices.reserve(plg_size);
    texture_indices.reserve(plg_size);

    for (int j = 0; j < plg_size; ++j) {
      int vertex_idx = fbx_mesh->GetPolygonVertex(i, j);
      // 装填face_vertex
      vertex_indices.push_back(vertex_idx);
      if (meta.GetNormalMappingMode() ==
          Metadata::ElementMappingMode::ByVertex) {
        normal_indices.push_back(vertex_idx);
      } else if (meta.GetNormalMappingMode() ==
                 Metadata::ElementMappingMode::ByFace) {
        normal_indices.push_back(i);
      } else if (meta.GetNormalMappingMode() ==
                 Metadata::ElementMappingMode::ByHalfedge) {
        int plg_vertex_idx = fbx_mesh->GetPolygonVertexIndex(i) + j;
        FV_ASSERT(plg_vertex_idx < normal_element->GetIndexArray().GetCount());
        if (normal_element->GetReferenceMode() ==
            fbxsdk::FbxLayerElement::eDirect)
          normal_indices.push_back(plg_vertex_idx);
        if (normal_element->GetReferenceMode() ==
            fbxsdk::FbxLayerElement::eIndexToDirect)
          normal_indices.push_back(
              normal_element->GetIndexArray().GetAt(plg_vertex_idx));
      }
      // 装填face_texture
      if (meta.GetTextureMappingMode() ==
          Metadata::ElementMappingMode::ByVertex)
        texture_indices.push_back(vertex_idx);
      else if (meta.GetTextureMappingMode() ==
               Metadata::ElementMappingMode::ByHalfedge)
        texture_indices.push_back(
            static_cast<std::size_t>(fbx_mesh->GetTextureUVIndex(i, j)));
    }
  }

  // 创建网格, 只包含基础的点面信息,
  // 不添加非流形面(通过map_plg_idx_to_manifoid_face_idx屏蔽)
  std::vector<int> map_plg_idx_to_manifoid_face_idx;
  dop::MeshOperation::BuildSurfaceMesh(vertices, face_vertex_indices, mesh,
                                       map_plg_idx_to_manifoid_face_idx);
  std::vector<int> map_manifoid_face_idx_to_plg_idx;
  map_manifoid_face_idx_to_plg_idx.reserve(mesh.num_faces());
  for (std::size_t i = 0; i < map_plg_idx_to_manifoid_face_idx.size(); ++i)
    if (map_plg_idx_to_manifoid_face_idx[i] != -1)
      map_manifoid_face_idx_to_plg_idx.push_back(i);

  if (meta.GetNormalMappingMode() != Metadata::ElementMappingMode::NONE) {
    auto face_vertex_normal =
        mesh.add_property_map<CGAL::SM_Face_index,
                              std::unordered_map<std::size_t, dop::Vec3II>>(
                "fv:normal")
            .first;
    for (const auto &face : mesh.faces()) {
      const auto &normal_indices =
          face_normal_indices[map_manifoid_face_idx_to_plg_idx[face.idx()]];
      const auto &vertex_indices =
          face_vertex_indices[map_manifoid_face_idx_to_plg_idx[face.idx()]];
      for (std::size_t i = 0; i < vertex_indices.size(); ++i)
        face_vertex_normal[face][vertex_indices[i]] =
            normal_array[normal_indices[i]];
    }
  }

  if (meta.GetTextureMappingMode() != Metadata::ElementMappingMode::NONE) {
    auto face_vertex_texture =
        mesh.add_property_map<CGAL::SM_Face_index,
                              std::unordered_map<std::size_t, dop::Point2II>>(
                "fv:texcoord")
            .first;
    for (const auto &face : mesh.faces()) {
      std::size_t face_idx = map_manifoid_face_idx_to_plg_idx[face.idx()];
      const auto &texture_indices = face_texture_indices[face_idx];
      const auto &vertex_indices = face_vertex_indices[face_idx];
      for (std::size_t i = 0; i < vertex_indices.size(); ++i)
        face_vertex_texture[face][vertex_indices[i]] =
            texture_array[texture_indices[i]];
    }
  }
}

void LoadFbxFile(const fs::path &filename, dop::SM_with_Meta &mesh_with_meta) {
  auto fbx_scene = LoadFbxScene(filename);
  if (!fbx_scene) {
    std::cerr << "Failed to load FBX scene from " << filename << std::endl;
    return;
  }

  auto transformed_scene = ApplyFbxSceneTransform(fbx_scene);
  if (!transformed_scene) {
    std::cerr << "Failed to apply transform to FBX scene" << std::endl;
    fbx_scene->Destroy();
    return;
  }

  // 如果返回的是新场景，销毁原场景
  if (transformed_scene != fbx_scene) {
    fbx_scene->Destroy();
    fbx_scene = transformed_scene;
  }

  auto merged_scene =
      MergeFbxNodesToOneNode(fbx_scene); // 将所有节点合并为一个节点
  if (!merged_scene) {
    std::cerr << "Failed to merge FBX nodes" << std::endl;
    fbx_scene->Destroy();
    return;
  }

  // 如果返回的是新场景，销毁原场景
  if (merged_scene != fbx_scene) {
    fbx_scene->Destroy();
    fbx_scene = merged_scene;
  }

  // 确保根节点存在且有子节点
  if (!fbx_scene->GetRootNode() ||
      fbx_scene->GetRootNode()->GetChildCount() == 0) {
    std::cerr << "FBX scene has no valid root node or child nodes" << std::endl;
    fbx_scene->Destroy();
    return;
  }

  fbxsdk::FbxMesh *fbx_mesh = fbx_scene->GetRootNode()->GetChild(0)->GetMesh();
  if (!fbx_mesh) {
    std::cerr << "No mesh found in the first child of root node" << std::endl;
    fbx_scene->Destroy();
    return;
  }

  CreateSurfaceMesh3FromFbxMesh(fbx_mesh, mesh_with_meta);

  fbx_scene->Destroy();
}

void SaveFbxMeshAsObj(const fs::path &filename, fbxsdk::FbxMesh *mesh) {
  if (!mesh) {
    std::cerr << "Invalid mesh pointer" << std::endl;
    return;
  }

  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filename << std::endl;
    return;
  }

  // 写入文件头
  file << "# OBJ file generated from FBX mesh" << std::endl;
  file << "# Vertices: " << mesh->GetControlPointsCount() << std::endl;
  file << "# Polygons: " << mesh->GetPolygonCount() << std::endl;
  file << std::endl;

  // 写入顶点数据
  const int num_vertices = mesh->GetControlPointsCount();
  const fbxsdk::FbxVector4 *control_points = mesh->GetControlPoints();

  for (int i = 0; i < num_vertices; ++i) {
    file << "v " << control_points[i][0] << " " << control_points[i][1] << " "
         << control_points[i][2] << std::endl;
  }
  file << std::endl;

  // 写入法线数据
  fbxsdk::FbxLayerElementNormal *normal_element = mesh->GetElementNormal();
  bool has_normals = (normal_element != nullptr &&
                      normal_element->GetDirectArray().GetCount() > 0);

  if (has_normals) {
    const fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector4> &normals =
        normal_element->GetDirectArray();

    for (int i = 0; i < normals.GetCount(); ++i) {
      const fbxsdk::FbxVector4 &n = normals[i];
      file << "vn " << n[0] << " " << n[1] << " " << n[2] << std::endl;
    }
    file << std::endl;
  }

  // 写入面数据
  const int num_faces = mesh->GetPolygonCount();

  for (int face_idx = 0; face_idx < num_faces; ++face_idx) {
    const int polygon_size = mesh->GetPolygonSize(face_idx);
    file << "f ";

    for (int vertex_pos = 0; vertex_pos < polygon_size; ++vertex_pos) {
      // 获取顶点索引
      int vertex_index =
          mesh->GetPolygonVertex(face_idx, vertex_pos) + 1; // OBJ索引从1开始

      // 获取法线索引
      int normal_index = -1;
      if (has_normals) {
        int polygon_vertex_idx =
            mesh->GetPolygonVertexIndex(face_idx) + vertex_pos;

        if (normal_element->GetMappingMode() ==
            FbxLayerElement::eByPolygonVertex) {
          if (normal_element->GetReferenceMode() == FbxLayerElement::eDirect) {
            normal_index = polygon_vertex_idx + 1;
          } else if (normal_element->GetReferenceMode() ==
                     FbxLayerElement::eIndexToDirect) {
            if (polygon_vertex_idx >= 0 &&
                polygon_vertex_idx <
                    normal_element->GetIndexArray().GetCount()) {
              normal_index =
                  normal_element->GetIndexArray().GetAt(polygon_vertex_idx) + 1;
            }
          }
        } else if (normal_element->GetMappingMode() ==
                   FbxLayerElement::eByControlPoint) {
          if (normal_element->GetReferenceMode() == FbxLayerElement::eDirect) {
            normal_index = vertex_index;
          } else if (normal_element->GetReferenceMode() ==
                     FbxLayerElement::eIndexToDirect) {
            if (vertex_index - 1 >= 0 &&
                vertex_index - 1 < normal_element->GetIndexArray().GetCount()) {
              normal_index =
                  normal_element->GetIndexArray().GetAt(vertex_index - 1) + 1;
            }
          }
        }
      }

      // 写入面顶点信息
      if (has_normals && normal_index > 0) {
        file << vertex_index << "//" << normal_index;
      } else {
        file << vertex_index;
      }

      if (vertex_pos < polygon_size - 1) {
        file << " ";
      }
    }
    file << std::endl;
  }

  file.close();
  std::cout << "FBX mesh successfully saved as OBJ: " << filename << std::endl;
}