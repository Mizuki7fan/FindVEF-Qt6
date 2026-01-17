#include "MainWindow.h"

#include <QActionGroup>
#include <QApplication>
#include <QDir>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QScreen>
#include <QStatusBar>
#include <QVBoxLayout>
#include <format>
#include <iostream>

#include "../../render/common/MergeSnapshot.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("FindVEF");

  centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  // 创建菜单栏
  setMenus();
  setPanels();
  setStatusBars();
  setConnects();

  //
  // 移除了loadPointEdgeSize调用

  actMenu11Layout->triggered();
  ResetWindowSize(0.8, 0.8);
}

MainWindow::~MainWindow() {}

void MainWindow::ResetWindowSize(double rate_width, double rate_height) {
  // qt6的特性
  QScreen *primary_screen = QGuiApplication::primaryScreen();
  if (primary_screen) {
    QRect primary_screen_geometry = primary_screen->geometry();
    int primary_screen_width = primary_screen_geometry.width();
    int primary_screen_height = primary_screen_geometry.height();

    int main_window_left_up_coord_x =
        static_cast<int>(0.1 * primary_screen_width);
    int main_window_left_up_coord_y =
        static_cast<int>(0.1 * primary_screen_height);
    int mian_window_width = static_cast<int>(rate_width * primary_screen_width);
    int mian_window_height =
        static_cast<int>(rate_height * primary_screen_height);

    this->setGeometry(main_window_left_up_coord_x, main_window_left_up_coord_y,
                      mian_window_width, mian_window_height);
  }
}

void MainWindow::setMenus() {
  QMenuBar *menuBar = new QMenuBar(this);
  setMenuBar(menuBar);

  QMenu *layoutMenu = new QMenu("视窗布局", this);
  menuBar->addMenu(layoutMenu);

  actMenu11Layout = new QAction("1x1 layout");
  actMenu11Layout->setCheckable(true);
  layoutMenu->addAction(actMenu11Layout);
  connect(actMenu11Layout, &QAction::triggered, this,
          &MainWindow::setMenu11Layout);

  actMenu12Layout = new QAction("1x2 layout");
  actMenu12Layout->setCheckable(true);
  layoutMenu->addAction(actMenu12Layout);
  connect(actMenu12Layout, &QAction::triggered, this,
          &MainWindow::setMenu12Layout);

  actMenu13Layout = new QAction("1x3 layout");
  actMenu13Layout->setCheckable(true);
  layoutMenu->addAction(actMenu13Layout);
  connect(actMenu13Layout, &QAction::triggered, this,
          &MainWindow::setMenu13Layout);

  actMenu14Layout = new QAction("1x4 layout");
  actMenu14Layout->setCheckable(true);
  layoutMenu->addAction(actMenu14Layout);
  connect(actMenu14Layout, &QAction::triggered, this,
          &MainWindow::setMenu14Layout);

  actMenu21Layout = new QAction("2x1 layout");
  actMenu21Layout->setCheckable(true);
  layoutMenu->addAction(actMenu21Layout);
  connect(actMenu21Layout, &QAction::triggered, this,
          &MainWindow::setMenu21Layout);

  actMenu22Layout = new QAction("2x2 layout");
  actMenu22Layout->setCheckable(true);
  layoutMenu->addAction(actMenu22Layout);
  connect(actMenu22Layout, &QAction::triggered, this,
          &MainWindow::setMenu22Layout);

  actMenu24Layout = new QAction("2x4 layout");
  actMenu24Layout->setCheckable(true);
  layoutMenu->addAction(actMenu24Layout);
  connect(actMenu24Layout, &QAction::triggered, this,
          &MainWindow::setMenu24Layout);
  QActionGroup *action_group_layout = new QActionGroup(this);
  action_group_layout->setExclusive(true); // 设置action互斥
  action_group_layout->addAction(actMenu11Layout);
  action_group_layout->addAction(actMenu12Layout);
  action_group_layout->addAction(actMenu13Layout);
  action_group_layout->addAction(actMenu14Layout);
  action_group_layout->addAction(actMenu21Layout);
  action_group_layout->addAction(actMenu22Layout);
  action_group_layout->addAction(actMenu24Layout);

  actMenu11Layout->setChecked(true);

  QMenu *sizeMenu = new QMenu("视窗大小", this);
  menuBar->addMenu(sizeMenu);

  actMenu88Size = new QAction("0.8 x 0.8 Size");
  actMenu88Size->setCheckable(true);
  sizeMenu->addAction(actMenu88Size);
  connect(actMenu88Size, &QAction::triggered, this, [=, this]() {
    if (actMenu88Size->isChecked())
      ResetWindowSize(0.8, 0.8);
  });

  actMenu48Size = new QAction("0.4 x 0.8 Size");
  actMenu48Size->setCheckable(true);
  sizeMenu->addAction(actMenu48Size);
  connect(actMenu48Size, &QAction::triggered, this, [=, this]() {
    if (actMenu48Size->isChecked())
      ResetWindowSize(0.4, 0.8);
  });

  QActionGroup *action_group_size = new QActionGroup(this);
  action_group_size->setExclusive(true);
  action_group_size->addAction(actMenu88Size);
  action_group_size->addAction(actMenu48Size);

  actMenu88Size->setChecked(true);

  // 添加光照模式菜单
  QMenu *lightingMenu = new QMenu("光照模式", this);
  menuBar->addMenu(lightingMenu);

  actLightingDefault = new QAction("默认光照 (推荐)");
  actLightingDefault->setCheckable(true);
  actLightingDefault->setShortcut(QKeySequence("Ctrl+1"));
  actLightingDefault->setStatusTip("使用改进的多光源配置，有效减少阴影问题");
  lightingMenu->addAction(actLightingDefault);
  connect(actLightingDefault, &QAction::triggered, this,
          &MainWindow::setLightingDefault);

  actLightingSoft = new QAction("柔和光照");
  actLightingSoft->setCheckable(true);
  actLightingSoft->setShortcut(QKeySequence("Ctrl+2"));
  actLightingSoft->setStatusTip("使用多个低强度光源，最大程度减少阴影");
  lightingMenu->addAction(actLightingSoft);
  connect(actLightingSoft, &QAction::triggered, this,
          &MainWindow::setLightingSoft);

  actLightingLegacy = new QAction("传统光照");
  actLightingLegacy->setCheckable(true);
  actLightingLegacy->setShortcut(QKeySequence("Ctrl+3"));
  actLightingLegacy->setStatusTip("原始单光源配置（用于对比）");
  lightingMenu->addAction(actLightingLegacy);
  connect(actLightingLegacy, &QAction::triggered, this,
          &MainWindow::setLightingLegacy);

  actLightingBright = new QAction("明亮光照");
  actLightingBright->setCheckable(true);
  actLightingBright->setShortcut(QKeySequence("Ctrl+4"));
  actLightingBright->setStatusTip("高环境光配置，适合查看细节");
  lightingMenu->addAction(actLightingBright);
  connect(actLightingBright, &QAction::triggered, this,
          &MainWindow::setLightingBright);

  QActionGroup *action_group_lighting = new QActionGroup(this);
  action_group_lighting->setExclusive(true);
  action_group_lighting->addAction(actLightingDefault);
  action_group_lighting->addAction(actLightingSoft);
  action_group_lighting->addAction(actLightingLegacy);
  action_group_lighting->addAction(actLightingBright);

  actLightingDefault->setChecked(true); // 默认选择改进的光照模式

  QMenu *functionmenu = new QMenu("功能", this);
  menuBar->addMenu(functionmenu);
  exportAsOBJ = new QAction("导出为OBJ");
  exportAsOBJ->setCheckable(true);
  functionmenu->addAction(exportAsOBJ);
  connect(exportAsOBJ, &QAction::triggered, this,
          [=, this]() { wPanel[0]->ExportMesh("obj", false); });
  exportAsOBJ_Triangle = new QAction("导出为三角化OBJ");
  exportAsOBJ_Triangle->setCheckable(true);
  functionmenu->addAction(exportAsOBJ_Triangle);
  connect(exportAsOBJ_Triangle, &QAction::triggered, this,
          [=, this]() { wPanel[0]->ExportMesh("obj", true); });
  exportAsOFF = new QAction("导出为OFF");
  exportAsOFF->setCheckable(true);
  functionmenu->addAction(exportAsOFF);
  connect(exportAsOFF, &QAction::triggered, this,
          [=, this]() { wPanel[0]->ExportMesh("off", false); });
  exportAsOFF_Triangle = new QAction("导出为三角化OFF");
  exportAsOFF_Triangle->setCheckable(true);
  functionmenu->addAction(exportAsOFF_Triangle);
  connect(exportAsOFF_Triangle, &QAction::triggered, this,
          [=, this]() { wPanel[0]->ExportMesh("off", true); });
}

void MainWindow::setStatusBars() {
  //
  statusBar = new QStatusBar(this);
  this->setStatusBar(statusBar);
  // 创建一个QLabel用于显示富文本
  statusLabel = new QLabel(this);
  statusLabel->setTextFormat(Qt::RichText); // 设置文本格式为富文本
                                            // 设置初始的富文本内容
  QString initialText =
      "<span style=\"color: grey; font-size: 16px;\">No Message.</span> ";
  statusLabel->setText(initialText);
  // 将QLabel添加到状态栏
  statusBar->addWidget(statusLabel);
}

void MainWindow::setPanels() {
  wPanel.fill(nullptr);
  for (std::size_t i = 0; i < NUM_MAX_PANEL; ++i) {
    wPanel[i] = new OpenGLFindVEFPanel(this, static_cast<int>(i));

    wPanel[i]->setObjectName(QString("wPanel %1").arg(i));
    wPanel[i]->setVisible(false);
    wPanel[i]->setAcceptDrops(true);
  }

  QScreen *controller_screen = nullptr;
  QList<QScreen *> all_screens = QGuiApplication::screens();

  // 检测系统屏幕数量, 如果是双屏, controll显示在第二屏幕上
  if (all_screens.size() == 1)
    controller_screen = all_screens[0];
  else
    controller_screen = all_screens[1];

  QRect controller_screen_geomerty = controller_screen->geometry();

  int controller_panel_width =
      static_cast<int>(0.2 * controller_screen_geomerty.width());
  int controller_panel_height =
      static_cast<int>(0.5 * controller_screen_geomerty.height());
  int controller_panel_leftup_pos_x = controller_screen_geomerty.left();
  // int controller_panel_leftup_pos_y =
  //     controller_screen_geomerty.top() +
  //     (controller_screen_geomerty.height() - controller_panel_height) / 2.0;
  // controller面板出现在靠近左上方的位置
  int controller_panel_leftup_pos_y =
      controller_screen_geomerty.top() +
      (controller_screen_geomerty.height() - controller_panel_height) / 20.0;
  wController = new FindVEFController();
  wController->setWindowTitle("Controller");
  wController->setGeometry(controller_panel_leftup_pos_x,
                           controller_panel_leftup_pos_y,
                           controller_panel_width, controller_panel_height);
  wController->raise();
  wController->show();
}

void MainWindow::setConnects() {
  // 确保wController已经创建并且其控件已经初始化
  if (!wController) {
    return;
  }

  // 检查关键控件是否已经创建
  if (!wController->pbSnap || !wController->cbDrawBnd ||
      !wController->cbDrawFindVEF || !wController->cbDrawInfo ||
      !wController->cbDrawNonManifold) {
    return;
  }

  connect(wController->pbSnap, &QPushButton::clicked, this,
          &MainWindow::snapshot);
  // 移除了点线Size相关的控件连接

  connect(wController->cbLockCameraConfigure, &QCheckBox::checkStateChanged,
          [=, this]() {
            for (int pid = 0; pid < NUM_MAX_PANEL; pid++)
              if (!wPanel[pid]->isHidden())
                wPanel[pid]->updateGL();
          });
  // 主tab的控件现在只影响第一个panel（panel 0），保持向后兼容性
  connect(wController->cbDrawPoints, &QCheckBox::checkStateChanged,
          [=, this]() {
            wController->getPanelSettings(0).drawPoints =
                wController->cbDrawPoints->isChecked();
            if (!wPanel[0]->isHidden())
              wPanel[0]->updateGL();
          });

  connect(wController->cbDrawFlat, &QCheckBox::checkStateChanged, [=, this]() {
    wController->getPanelSettings(0).drawFlat =
        wController->cbDrawFlat->isChecked();
    if (!wPanel[0]->isHidden())
      wPanel[0]->updateGL();
  });

  connect(wController->cbDrawFlatWithNormal, &QCheckBox::checkStateChanged,
          [=, this]() {
            wController->getPanelSettings(0).drawFlatWithNormal =
                wController->cbDrawFlatWithNormal->isChecked();
            if (!wPanel[0]->isHidden())
              wPanel[0]->updateGL();
          });

  connect(wController->cbDrawNonManifold, &QCheckBox::checkStateChanged,
          [=, this]() {
            wController->getPanelSettings(0).drawNonManifold =
                wController->cbDrawNonManifold->isChecked();
            if (!wPanel[0]->isHidden())
              wPanel[0]->updateGL();
          });

  connect(wController->cbDrawWireframe, &QCheckBox::checkStateChanged,

          [=, this]() {
            wController->getPanelSettings(0).drawWireframe =
                wController->cbDrawWireframe->isChecked();
            if (!wPanel[0]->isHidden())
              wPanel[0]->updateGL();
          });

  connect(wController->cbDrawBnd, &QCheckBox::checkStateChanged, [=, this]() {
    wController->getPanelSettings(0).drawBnd =
        wController->cbDrawBnd->isChecked();
    if (!wPanel[0]->isHidden())
      wPanel[0]->updateGL();
  });
  connect(wController->cbDrawFindVEF, &QCheckBox::checkStateChanged,
          [=, this]() {
            wController->getPanelSettings(0).drawFindVEF =
                wController->cbDrawFindVEF->isChecked();
            if (!wPanel[0]->isHidden())
              wPanel[0]->updateGL();
          });
  // 移除了EnableLighting控件连接

  connect(wController->cbDrawModelFullPath, &QCheckBox::checkStateChanged,
          [=, this]() {
            wController->getPanelSettings(0).drawModelFullPath =
                wController->cbDrawModelFullPath->isChecked();
            if (!wPanel[0]->isHidden())
              wPanel[0]->updateGL();
          });
  connect(wController->cbDrawModelName, &QCheckBox::checkStateChanged,
          [=, this]() {
            wController->getPanelSettings(0).drawModelName =
                wController->cbDrawModelName->isChecked();
            if (!wPanel[0]->isHidden())
              wPanel[0]->updateGL();
          });

  connect(wController->cbDrawInfo, &QCheckBox::checkStateChanged, [=, this]() {
    wController->getPanelSettings(0).drawInfo =
        wController->cbDrawInfo->isChecked();
    if (!wPanel[0]->isHidden())
      wPanel[0]->updateGL();
  });
  connect(wController->cbDrawMeshInfo, &QCheckBox::checkStateChanged,
          [=, this]() {
            wController->getPanelSettings(0).drawMeshInfo =
                wController->cbDrawMeshInfo->isChecked();
            if (!wPanel[0]->isHidden())
              wPanel[0]->updateGL();
          });
}

void MainWindow::closeEvent(QCloseEvent *) {

  // 移除了savePointEdgeSize调用

  wController->close();
  exit(0);
}

void MainWindow::syncCameraConfiguration(
    const int &k, const BasePanel::CameraConfiguration &camera_configuration) {

  if (!wController->cbLockCameraConfigure->isChecked())
    return;
  for (std::size_t i = 0; i < wPanel.size(); i++) {
    if (i == k)
      continue;
    wPanel[i]->camera = camera_configuration;
    wPanel[i]->updateGL();
  }
}

void MainWindow::snapshot() {
  //
  QImage image_all;
  switch (layout_status) {
  case LayoutStatus::LAYOUT11:
    image_all = wPanel[0]->SnapShot();
    break;
  case LayoutStatus::LAYOUT12:
    image_all = MergeLayout1x2(wPanel[0]->SnapShot(), wPanel[1]->SnapShot());
    break;
  case LayoutStatus::LAYOUT13:
    image_all = MergeLayout1x3(wPanel[0]->SnapShot(), wPanel[1]->SnapShot(),
                               wPanel[2]->SnapShot());
    break;
  case LayoutStatus::LAYOUT14:
    image_all = MergeLayout1x4(wPanel[0]->SnapShot(), wPanel[1]->SnapShot(),
                               wPanel[2]->SnapShot(), wPanel[3]->SnapShot());
    break;
  case LayoutStatus::LAYOUT21:
    image_all = MergeLayout2x1(wPanel[0]->SnapShot(), wPanel[1]->SnapShot());
    break;
  case LayoutStatus::LAYOUT22:
    image_all = MergeLayout2x2(wPanel[0]->SnapShot(), wPanel[1]->SnapShot(),
                               wPanel[2]->SnapShot(), wPanel[3]->SnapShot());
    break;
  case LayoutStatus::LAYOUT24:
    image_all = MergeLayout2x4(wPanel[0]->SnapShot(), wPanel[1]->SnapShot(),
                               wPanel[2]->SnapShot(), wPanel[3]->SnapShot(),
                               wPanel[4]->SnapShot(), wPanel[5]->SnapShot(),
                               wPanel[6]->SnapShot(), wPanel[7]->SnapShot());
    break;
  case LayoutStatus::NONE:
    break;
  default:
    break;
  }
  //
  QApplication::clipboard()->setImage(image_all);
  QString Text = "<span style=\"color: red; font-size: "
                 "16px;\">截屏已经保存到缓冲区</span> ";
  statusLabel->setText(Text);
}

void MainWindow::clearLayout() {
  if (layout) {
    QLayoutItem *item;

    while ((item = layout->takeAt(0)) != nullptr) {
      if (item->widget()) {
        std::cout << std::format("Hiding: {}",
                                 item->widget()->objectName().toStdString())
                  << std::endl;
        item->widget()->setVisible(false);
      }
      delete item;
    }
    delete layout;
    layout = nullptr;
  }

  // 更新子tab的显示状态
  wController->updatePanelTabsVisibility();
}

void MainWindow::setMenu11Layout() {
  clearLayout();

  layout = new QVBoxLayout;
  centralWidget->setLayout(layout);
  layout->addWidget(wPanel[0]);
  wPanel[0]->setVisible(true);

  wController->cbLockCameraConfigure->hide();
  layout_status = LAYOUT11;

  // 更新子tab的显示状态
  wController->updatePanelTabsVisibility();
}

void MainWindow::setMenu12Layout() {
  clearLayout();

  layout = new QHBoxLayout;
  centralWidget->setLayout(layout);
  layout->addWidget(wPanel[0]);
  layout->addWidget(wPanel[1]);
  wPanel[0]->setVisible(true);
  wPanel[1]->setVisible(true);
  // 此处设置Panel的名字
  //  wPanel[0]->setPanelName("master");
  //  wPanel[1]->setPanelName("tianyuzhu");

  wController->cbLockCameraConfigure->show();
  layout_status = LAYOUT12;

  // 更新子tab的显示状态
  wController->updatePanelTabsVisibility();
}

void MainWindow::setMenu13Layout() {
  clearLayout();

  layout = new QHBoxLayout;
  centralWidget->setLayout(layout);
  layout->addWidget(wPanel[0]);
  layout->addWidget(wPanel[1]);
  layout->addWidget(wPanel[2]);
  wPanel[0]->setVisible(true);
  wPanel[1]->setVisible(true);
  wPanel[2]->setVisible(true);

  wController->cbLockCameraConfigure->show();
  layout_status = LAYOUT13;

  // 更新子tab的显示状态
  wController->updatePanelTabsVisibility();
}

void MainWindow::setMenu14Layout() {
  clearLayout();

  layout = new QHBoxLayout;
  centralWidget->setLayout(layout);
  layout->addWidget(wPanel[0]);
  layout->addWidget(wPanel[1]);
  layout->addWidget(wPanel[2]);
  layout->addWidget(wPanel[3]);
  wPanel[0]->setVisible(true);
  wPanel[1]->setVisible(true);
  wPanel[2]->setVisible(true);
  wPanel[3]->setVisible(true);

  wController->cbLockCameraConfigure->show();
  layout_status = LAYOUT14;

  // 更新子tab的显示状态
  wController->updatePanelTabsVisibility();
}

void MainWindow::setMenu21Layout() {
  clearLayout();

  layout = new QVBoxLayout;
  centralWidget->setLayout(layout);
  layout->addWidget(wPanel[0]);
  layout->addWidget(wPanel[1]);
  wPanel[0]->setVisible(true);
  wPanel[1]->setVisible(true);

  wController->cbLockCameraConfigure->show();
  layout_status = LAYOUT21;

  // 更新子tab的显示状态
  wController->updatePanelTabsVisibility();
}

void MainWindow::setMenu22Layout() {
  clearLayout();

  QGridLayout *grid_layout = new QGridLayout;
  layout = grid_layout;
  centralWidget->setLayout(grid_layout);
  grid_layout->addWidget(wPanel[0], 0, 0);
  grid_layout->addWidget(wPanel[1], 0, 1);
  grid_layout->addWidget(wPanel[2], 1, 0);
  grid_layout->addWidget(wPanel[3], 1, 1);
  wPanel[0]->setVisible(true);
  wPanel[1]->setVisible(true);
  wPanel[2]->setVisible(true);
  wPanel[3]->setVisible(true);

  wController->cbLockCameraConfigure->show();
  layout_status = LAYOUT22;

  // 更新子tab的显示状态
  wController->updatePanelTabsVisibility();
}

void MainWindow::setMenu24Layout() {
  clearLayout();

  QGridLayout *grid_layout = new QGridLayout;
  layout = grid_layout;
  centralWidget->setLayout(grid_layout);
  grid_layout->addWidget(wPanel[0], 0, 0);
  grid_layout->addWidget(wPanel[1], 0, 1);
  grid_layout->addWidget(wPanel[2], 0, 2);
  grid_layout->addWidget(wPanel[3], 0, 3);
  grid_layout->addWidget(wPanel[4], 1, 0);
  grid_layout->addWidget(wPanel[5], 1, 1);
  grid_layout->addWidget(wPanel[6], 1, 2);
  grid_layout->addWidget(wPanel[7], 1, 3);
  wPanel[0]->setVisible(true);
  wPanel[1]->setVisible(true);
  wPanel[2]->setVisible(true);
  wPanel[3]->setVisible(true);
  wPanel[4]->setVisible(true);
  wPanel[5]->setVisible(true);
  wPanel[6]->setVisible(true);
  wPanel[7]->setVisible(true);

  wController->cbLockCameraConfigure->show();
  layout_status = LAYOUT24;

  // 更新子tab的显示状态
  wController->updatePanelTabsVisibility();
}

// ==================== 光照模式切换函数 ====================

void MainWindow::setLightingDefault() {
  // 设置所有可见Panel为默认光照模式
  for (int i = 0; i < NUM_MAX_PANEL; ++i) {
    if (!wPanel[i]->isHidden()) {
      wPanel[i]->setLightingMode(OpenGLFindVEFPanel::LIGHTING_DEFAULT);
    }
  }
  statusLabel->setText(
      "光照模式: <b>默认光照</b> - 改进的多光源配置，有效减少阴影问题");
}

void MainWindow::setLightingSoft() {
  // 设置所有可见Panel为柔和光照模式
  for (int i = 0; i < NUM_MAX_PANEL; ++i) {
    if (!wPanel[i]->isHidden()) {
      wPanel[i]->setLightingMode(OpenGLFindVEFPanel::LIGHTING_SOFT);
    }
  }
  statusLabel->setText(
      "光照模式: <b>柔和光照</b> - 使用多个低强度光源，最大程度减少阴影");
}

void MainWindow::setLightingLegacy() {
  // 设置所有可见Panel为传统光照模式
  for (int i = 0; i < NUM_MAX_PANEL; ++i) {
    if (!wPanel[i]->isHidden()) {
      wPanel[i]->setLightingMode(OpenGLFindVEFPanel::LIGHTING_LEGACY);
    }
  }
  statusLabel->setText(
      "光照模式: <b>传统光照</b> - 原始单光源配置（用于对比效果）");
}

void MainWindow::setLightingBright() {
  // 设置所有可见Panel为明亮光照模式
  for (int i = 0; i < NUM_MAX_PANEL; ++i) {
    if (!wPanel[i]->isHidden()) {
      wPanel[i]->setLightingMode(OpenGLFindVEFPanel::LIGHTING_BRIGHT);
    }
  }
  statusLabel->setText(
      "光照模式: <b>明亮光照</b> - 高环境光配置，适合查看模型细节");
}

// 移除了savePointEdgeSize和loadPointEdgeSize函数，因为不再需要点线大小设置
