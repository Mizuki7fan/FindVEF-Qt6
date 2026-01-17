#pragma once

#include "../../core/geometry/metadata.h"
#include "../../core/geometry/surfacemesh.h"
#include "../../ui/windows/MainWindow.h"
#include "../../utils/FindVEFHandler.h"
#include "../common/BasePanel.h"

// 参考MeshViewerWidget, 主要用来绘制Mesh, 处理Mesh相关的逻辑
class MainWindow;
struct PanelDrawSettings;
class OpenGLFindVEFPanel : public BasePanel {

public:
  OpenGLFindVEFPanel(MainWindow *parent, int panel_id);

  ~OpenGLFindVEFPanel();

  void setPanelName(QString str) { panel_name = str; };

  void clearInfo();
  void UpdateCamera();
  QImage SnapShot();

  void jumpToVEF(std::string str);

  // 光照模式切换功能 - 用于解决阴影问题
  enum LightingMode {
    LIGHTING_DEFAULT, // 默认光照（改进后的多光源）
    LIGHTING_SOFT,    // 柔和光照（减少阴影对比度）
    LIGHTING_LEGACY,  // 传统光照（原始单光源）
    LIGHTING_BRIGHT   // 明亮光照（高环境光）
  };
  void setLightingMode(LightingMode mode);
  LightingMode getCurrentLightingMode() const { return currentLightingMode; }

  // 使用包装类，mesh和metadata已经绑定在一起
  dop::SM_with_Meta mesh_with_meta;

  enum DrawOption { // 点, 线框, 面, 带法向信息的面
    POINTS,
    WIREFRAME,
    FLAT,
    FLAT_WITH_NORMAL,
    NONE
  };
  DrawOption drawmode = FLAT;

private:
  void draw() override;

  void draw_points(void) const;
  void draw_flat(void) const;
  void draw_flat_with_normal(void) const;
  void draw_wireframe(void) const;
  void draw_bnd_glu(void) const;
  void draw_findvef(void) const;
  void draw_non_manifold(void) const; // 新增：绘制非流形面
  void draw_mesh_info(void) const;

  void draw_moji(void);

  void update_draw_option();
  bool EnableLighting = true; // 默认开启光照以提供更好的3D视觉效果

  bool DrawBnd = true;
  bool DrawFindVEF = false;
  bool DrawModelName = false;
  bool DrawModelFullPath = false;
  bool DrawInfo = true;

  // 新增：文件信息显示选项
  bool DrawFileSizeFormat = true; // 合并文件大小和格式显示，默认开启
  bool DrawMeshInfo = false;      // 显示顶点和面的索引，默认关闭

  // 绘制模式状态
  bool DrawPoints = false;
  bool DrawWireframe = true;
  bool DrawFlat = true;
  bool DrawFlatWithNormal = false;
  bool DrawNonManifold = true; // 新增：非流形面绘制选项，默认开启
  dop::Point3II ptMin, ptMax;

  void dropEvent(QDropEvent *) override;
  void dragEnterEvent(QDragEnterEvent *) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

  // FindVEF相关
  FindVEFHandler findvef_info;
  // Texture相关

  // 光照模式管理
  LightingMode currentLightingMode = LIGHTING_DEFAULT;
  void applyLightingMode(LightingMode mode) const;

public: // 菜单功能
  void ExportMesh(std::string format, bool triangulate = false);

protected:
  void initializeGL();

  MainWindow *parent;
  const int panel_id;
  QString panel_name = "";
};