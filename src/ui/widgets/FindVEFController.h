#pragma once
#include "../../utils/FindVEFHandler.h"
#include "Controller.h"
#include <QStack>
#include <cstddef>

#define NUM_MAX_PANEL 8

// 每个panel的独立绘制参数
struct PanelDrawSettings {
  // 网格绘制选项
  bool drawPoints = false;
  bool drawWireframe = true;
  bool drawFlat = false;
  bool drawFlatWithNormal = true;

  bool drawNonManifold = true; // 默认开启

  // 其他绘制选项
  bool drawBnd = false;

  bool drawFindVEF = true;
  bool drawInfo = true;

  bool drawModelName = true;
  bool drawModelFullPath = false;

  // 新增：文件信息显示选项
  bool drawFileSizeFormat = true; // 合并文件大小和格式显示，默认开启

  // 新增：网格信息显示选项
  bool drawMeshInfo = false; // 显示顶点和面的索引，默认关闭
};

class FindVEFController : public Controller {
  Q_OBJECT
public:
  FindVEFController(QWidget *_parent = 0);
  ~FindVEFController();

  void setGlobalLayout();
  void setConnects();

private:
  // 设置FindVEF标签页
  QWidget *setupFindVEFTab();
  // 设置folder标签页
  QWidget *setupFolderTab();
  // 设置单个panel的子tab
  QWidget *setupPanelSubTab(int panelId);

  // 目录历史管理
  QStack<QString> directoryHistory;                      // 目录历史栈
  QPushButton *pbBackDirectory = nullptr;                // 回退按钮引用
  QLabel *lblCurrentFolder = nullptr;                    // 当前文件夹标签引用
  void pushCurrentDirectory(const QString &currentPath); // 将当前目录推入历史栈
  void updateCurrentFolderLabel(const QString &path); // 更新当前文件夹标签显示

public:
  std::array<CustomSeperator *, 10> Seperator = {
      {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
       nullptr, nullptr}};

  QLineEdit *leJump = nullptr;
  QPushButton *pbJump = nullptr;

  QCheckBox *cbLockCameraConfigure = nullptr; // 锁定相机参数
  QPushButton *pbSnap = nullptr;

  // 网格绘制选项
  QCheckBox *cbDrawPoints = nullptr;
  QCheckBox *cbDrawWireframe = nullptr;
  QCheckBox *cbDrawFlat = nullptr;
  QCheckBox *cbDrawFlatWithNormal = nullptr;
  QButtonGroup *bgDrawFace = nullptr; // 用于管理绘制面选项的互斥逻辑

  QCheckBox *cbDrawNonManifold = nullptr;

  QCheckBox *cbDrawBnd = nullptr;

  QCheckBox *cbDrawFindVEF = nullptr;
  QCheckBox *cbDrawModelName = nullptr;
  QCheckBox *cbDrawModelFullPath = nullptr;
  QButtonGroup *bgDrawModelInfo = nullptr; // 用于管理模型信息显示选项的互斥逻辑
  QCheckBox *cbDrawInfo = nullptr;

  // 新增：文件信息控件
  QCheckBox *cbDrawFileSizeFormat = nullptr; // 合并文件大小和格式显示

  // 新增：网格信息控件
  QCheckBox *cbDrawMeshInfo = nullptr; // 显示顶点和面的索引

  // 子tab相关成员变量
  QTabWidget *panelTabWidget = nullptr; // 用于管理8个panel的子tab
  std::array<QWidget *, NUM_MAX_PANEL> panelSubTabs = {nullptr}; // 8个子tab
  void updatePanelTabsVisibility(); // 更新子tab的显示状态

  // 每个panel的独立绘制参数
  std::array<PanelDrawSettings, NUM_MAX_PANEL> panelSettings;

  // 获取指定panel的绘制参数
  const PanelDrawSettings &getPanelSettings(int panelId) const;
  PanelDrawSettings &getPanelSettings(int panelId);
};
