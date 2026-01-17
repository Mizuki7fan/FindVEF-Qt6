#pragma once
#include <QMainWindow>
#include <QtWidgets>
#include <qaction.h>

#include "../../render/opengl1/OpenGLFindVEFPanel.h"

#include "../../render/common/BasePanel.h"
#include "../widgets/FindVEFController.h"

#define NUM_MAX_PANEL 8

class FindVEFController;
class OpenGLFindVEFPanel;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = 0);
  ~MainWindow();
  void syncCameraConfiguration(
      const int &k, const BasePanel::CameraConfiguration &camera_configuration);

  enum LayoutStatus {
    LAYOUT11,
    LAYOUT12,
    LAYOUT13,
    LAYOUT14,
    LAYOUT21,
    LAYOUT22,
    LAYOUT24,
    NONE
  };
  LayoutStatus layout_status = NONE;

  void snapshot();

private:
  void setMenus();
  QStatusBar *statusBar = nullptr;
  QLabel *statusLabel = nullptr; // 显示富文本
  void setStatusBars();
  void setPanels();
  void setConnects();

protected:
  void closeEvent(QCloseEvent *event); // 关闭事件;

private:
  QWidget *wMain; // 主界面

  QAction *actMenu11Layout;
  QAction *actMenu12Layout;
  QAction *actMenu13Layout;
  QAction *actMenu14Layout;
  QAction *actMenu21Layout;
  QAction *actMenu22Layout;
  QAction *actMenu24Layout;
  QAction *actMenu88Size;
  QAction *actMenu48Size;

  // 光照模式相关Action
  QAction *actLightingDefault;
  QAction *actLightingSoft;
  QAction *actLightingLegacy;
  QAction *actLightingBright;

  QAction *exportAsOBJ;
  QAction *exportAsOBJ_Triangle;
  QAction *exportAsOFF;
  QAction *exportAsOFF_Triangle;

  QWidget *centralWidget;

  QLayout *layout = nullptr;

  void clearLayout();

private slots:
  void setMenu11Layout();
  void setMenu12Layout();
  void setMenu13Layout();
  void setMenu14Layout();
  void setMenu21Layout();
  void setMenu22Layout();
  void setMenu24Layout();

  // 光照模式切换槽函数
  void setLightingDefault();
  void setLightingSoft();
  void setLightingLegacy();
  void setLightingBright();

private:
  // 移除了savePointEdgeSize和loadPointEdgeSize函数声明

public slots:
  void ResetWindowSize(double rate_width = 0.8, double rate_height = 0.8);

public:
  std::array<OpenGLFindVEFPanel *, NUM_MAX_PANEL> wPanel;

  FindVEFController *wController;
  std::vector<std::string> path_key_word;
};