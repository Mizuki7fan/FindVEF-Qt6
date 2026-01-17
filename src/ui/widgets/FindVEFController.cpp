#include "FindVEFController.h"
#include "../windows/MainWindow.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QTreeView>
#include <QVBoxLayout>

FindVEFController::FindVEFController(QWidget *) {
  Controller::Do();
  // 确保控件创建完成后再次调用setConnects
  setConnects();
}
FindVEFController::~FindVEFController() {
  // 清空历史栈
  directoryHistory.clear();

  // 确保所有信号槽连接被正确断开（Qt会自动处理，但显式清理更安全）
  if (pbBackDirectory) {
    pbBackDirectory->disconnect();
  }
  if (lblCurrentFolder) {
    lblCurrentFolder->disconnect();
  }
}

void FindVEFController::setGlobalLayout() {
  // 创建标签页控件
  QTabWidget *tabWidget = new QTabWidget(this);

  // 设置FindVEF标签页
  QWidget *findVEFTab = setupFindVEFTab();
  tabWidget->addTab(findVEFTab, "FindVEF");

  // 设置folder标签页
  QWidget *folderTab = setupFolderTab();
  tabWidget->addTab(folderTab, "Folder");

  // 将标签页控件添加到父类的globalLayout中
  globalLayout->addWidget(tabWidget);
}

QWidget *FindVEFController::setupFindVEFTab() {
  QWidget *findVEFTab = new QWidget();
  QVBoxLayout *findVEFLayout = new QVBoxLayout(findVEFTab);

  // 创建子tab控件
  panelTabWidget = new QTabWidget();
  panelTabWidget->setVisible(false); // 默认隐藏，只有多个panel时才显示

  // 创建8个子tab
  const QString panelNames[] = {"①", "②", "③", "④", "⑤", "⑥", "⑦", "⑧"};
  for (int i = 0; i < NUM_MAX_PANEL; i++) {
    panelSubTabs[i] = setupPanelSubTab(i);
    panelTabWidget->addTab(panelSubTabs[i], panelNames[i]);
  }

  // 将子tab控件添加到布局中
  findVEFLayout->addWidget(panelTabWidget);

  // 设置Seperator - 只保留截图模块
  std::size_t sep_id = 0;
  Seperator[0] = new CustomSeperator("截图");
  Seperator[1] = new CustomSeperator("几何信息");
  Seperator[2] = new CustomSeperator("FindVEF绘制");
  Seperator[3] = new CustomSeperator("元信息");

  // Jump相关控件（保持不变）
  leJump = new QLineEdit();
  pbJump = new QPushButton("Jump");

  // 截图相关控件
  cbLockCameraConfigure = new QCheckBox("锁定相机参数");
  cbLockCameraConfigure->setChecked(true);
  pbSnap = new QPushButton("Snapshot");

  // 几何信息模块
  cbDrawPoints = new QCheckBox("绘制顶点");
  cbDrawPoints->setChecked(false);
  cbDrawWireframe = new QCheckBox("绘制线框");
  cbDrawWireframe->setChecked(true);
  cbDrawFlat = new QCheckBox("绘制面");
  cbDrawFlat->setChecked(false);
  cbDrawFlatWithNormal = new QCheckBox("绘制面(法向)");
  cbDrawFlatWithNormal->setChecked(true);
  cbDrawNonManifold = new QCheckBox("绘制非流形面");
  cbDrawNonManifold->setChecked(true); // 默认开启
  cbDrawBnd = new QCheckBox("绘制边界");
  cbDrawBnd->setChecked(false);

  cbDrawInfo = new QCheckBox("点/线/面数");
  cbDrawInfo->setChecked(true);

  // FindVEF绘制模块
  cbDrawFindVEF = new QCheckBox("绘制FindVEF");
  cbDrawFindVEF->setChecked(true);

  // 元信息模块
  cbDrawModelName = new QCheckBox("文件名");
  cbDrawModelName->setChecked(true);
  cbDrawModelFullPath = new QCheckBox("文件完整路径");
  cbDrawModelFullPath->setChecked(false);
  cbDrawFileSizeFormat = new QCheckBox("文件大小/格式");
  cbDrawFileSizeFormat->setChecked(true); // 默认开启

  // 网格信息模块
  cbDrawMeshInfo = new QCheckBox("顶点/面索引");
  cbDrawMeshInfo->setChecked(false); // 默认关闭

  // 同步主tab控件与第一个panel的设置

  cbDrawPoints->setChecked(panelSettings[0].drawPoints);
  cbDrawWireframe->setChecked(panelSettings[0].drawWireframe);
  cbDrawFlat->setChecked(panelSettings[0].drawFlat);
  cbDrawFlatWithNormal->setChecked(panelSettings[0].drawFlatWithNormal);
  cbDrawNonManifold->setChecked(panelSettings[0].drawNonManifold);
  cbDrawBnd->setChecked(panelSettings[0].drawBnd);

  cbDrawFindVEF->setChecked(panelSettings[0].drawFindVEF);
  cbDrawInfo->setChecked(panelSettings[0].drawInfo);
  cbDrawModelName->setChecked(panelSettings[0].drawModelName);
  cbDrawModelFullPath->setChecked(panelSettings[0].drawModelFullPath);
  cbDrawFileSizeFormat->setChecked(panelSettings[0].drawFileSizeFormat);
  cbDrawMeshInfo->setChecked(panelSettings[0].drawMeshInfo);

  // 创建按钮组来管理绘制面选项的互斥逻辑

  bgDrawFace = new QButtonGroup(this);
  bgDrawFace->addButton(cbDrawFlat);
  bgDrawFace->addButton(cbDrawFlatWithNormal);
  bgDrawFace->setExclusive(false); // 允许都不选中

  // 创建按钮组来管理模型信息显示选项的互斥逻辑
  bgDrawModelInfo = new QButtonGroup(this);
  bgDrawModelInfo->addButton(cbDrawModelName);
  bgDrawModelInfo->addButton(cbDrawModelFullPath);
  bgDrawModelInfo->setExclusive(false); // 允许都不选中

  // 创建几何信息组框
  QGroupBox *gbGeometryInfo = new QGroupBox("");

  gbGeometryInfo->setStyleSheet(
      "QGroupBox { border: 1px solid gray; border-radius: 5px; margin-top: "
      "0px; }"
      "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 "
      "0px 0 0px; }");
  QVBoxLayout *gbGeometryInfoLayout = new QVBoxLayout(gbGeometryInfo);
  gbGeometryInfoLayout->addWidget(cbDrawPoints);
  gbGeometryInfoLayout->addWidget(cbDrawWireframe);
  gbGeometryInfoLayout->addWidget(cbDrawFlat);
  gbGeometryInfoLayout->addWidget(cbDrawFlatWithNormal);
  gbGeometryInfoLayout->addWidget(cbDrawNonManifold);
  gbGeometryInfoLayout->addWidget(cbDrawBnd);

  gbGeometryInfoLayout->addWidget(cbDrawInfo);

  // 创建FindVEF绘制组框
  QGroupBox *gbFindVEFDraw = new QGroupBox("");

  gbFindVEFDraw->setStyleSheet(
      "QGroupBox { border: 1px solid gray; border-radius: 5px; margin-top: "
      "0px; }"
      "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 "
      "0px 0 0px; }");
  QVBoxLayout *gbFindVEFDrawLayout = new QVBoxLayout(gbFindVEFDraw);
  gbFindVEFDrawLayout->addWidget(cbDrawFindVEF);

  // 创建元信息组框
  QGroupBox *gbMetaInfo = new QGroupBox("");

  gbMetaInfo->setStyleSheet(
      "QGroupBox { border: 1px solid gray; border-radius: 5px; margin-top: "
      "0px; }"
      "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 "
      "0px 0 0px; }");
  QVBoxLayout *gbMetaInfoLayout = new QVBoxLayout(gbMetaInfo);
  gbMetaInfoLayout->addWidget(cbDrawModelName);
  gbMetaInfoLayout->addWidget(cbDrawModelFullPath);
  gbMetaInfoLayout->addWidget(cbDrawFileSizeFormat);
  gbMetaInfoLayout->addWidget(cbDrawMeshInfo);

  // 添加控件到主布局

  findVEFLayout->addWidget(leJump);
  findVEFLayout->addWidget(pbJump);
  findVEFLayout->addWidget(Seperator[sep_id++]);
  findVEFLayout->addWidget(cbLockCameraConfigure);
  findVEFLayout->addWidget(pbSnap);
  findVEFLayout->addWidget(Seperator[sep_id++]);
  findVEFLayout->addWidget(gbGeometryInfo);
  findVEFLayout->addWidget(Seperator[sep_id++]);
  findVEFLayout->addWidget(gbFindVEFDraw);
  findVEFLayout->addWidget(Seperator[sep_id++]);
  findVEFLayout->addWidget(gbMetaInfo);
  findVEFLayout->addStretch();

  return findVEFTab;
}

QWidget *FindVEFController::setupFolderTab() {
  QWidget *folderTab = new QWidget();
  QVBoxLayout *folderLayout = new QVBoxLayout(folderTab);

  // 设置默认目录为项目根目录下的ast文件夹
  QString projectRoot = QCoreApplication::applicationDirPath();
  // 从bin目录回到项目根目录
  QDir dir(projectRoot);
  dir.cdUp(); // 从bin目录回到上级目录
  dir.cdUp(); // 从run目录回到项目根目录
  QString astPath = dir.absoluteFilePath("ast/FindVEF");

  // 如果ast目录不存在，则创建它
  if (!QDir(astPath).exists()) {
    QDir().mkpath(astPath);
  }

  // 第一行：当前目录信息
  lblCurrentFolder = new QLabel();
  lblCurrentFolder->setStyleSheet("color: blue;");
  lblCurrentFolder->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
  lblCurrentFolder->setWordWrap(false); // 禁用自动换行
  lblCurrentFolder->setTextFormat(Qt::PlainText);

  // 设置初始路径显示
  updateCurrentFolderLabel(astPath);

  folderLayout->addWidget(lblCurrentFolder);

  // 第二行：按钮
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  QPushButton *pbParentFolder = new QPushButton("上级目录");
  QPushButton *pbOpenFolder = new QPushButton("选定目录");
  pbBackDirectory = new QPushButton("回退目录");
  pbBackDirectory->setEnabled(false); // 初始状态禁用
  buttonLayout->addWidget(pbParentFolder);
  buttonLayout->addWidget(pbOpenFolder);
  buttonLayout->addWidget(pbBackDirectory);
  folderLayout->addLayout(buttonLayout);

  // 初始化目录历史栈
  directoryHistory.clear();

  // 第三行：过滤输入框
  QLineEdit *leFilter = new QLineEdit();
  leFilter->setPlaceholderText("输入关键字过滤...");
  folderLayout->addWidget(leFilter);

  // 文件夹视图
  QFileSystemModel *folderModel = new QFileSystemModel();
  folderModel->setRootPath(astPath);
  folderModel->setNameFilters(QStringList() << "*.obj" << "*.findvef" << "*.off"
                                            << "*.fbx" << "*.vtk");
  folderModel->setNameFilterDisables(false);

  QTreeView *folderView = new QTreeView();
  folderView->setModel(folderModel);
  folderView->setRootIndex(folderModel->index(astPath));
  folderView->setHeaderHidden(true);
  folderView->hideColumn(1);
  folderView->hideColumn(2);
  folderView->hideColumn(3);
  folderView->setSortingEnabled(true);

  // 启用拖拽功能
  folderView->setDragEnabled(true);
  folderView->setDragDropMode(QAbstractItemView::DragOnly);
  folderView->setDefaultDropAction(Qt::CopyAction);

  // 双击跳转
  QObject::connect(folderView, &QTreeView::doubleClicked,
                   [this, folderModel, folderView](const QModelIndex &index) {
                     if (folderModel->isDir(index)) {
                       QString currentPath =
                           folderModel->filePath(folderView->rootIndex());
                       QString newPath = folderModel->filePath(index);

                       // 推入当前目录到历史栈
                       pushCurrentDirectory(currentPath);

                       folderView->setRootIndex(index);
                       updateCurrentFolderLabel(newPath);
                     }
                   });

  // 返回上级目录
  QObject::connect(
      pbParentFolder, &QPushButton::clicked, [this, folderModel, folderView]() {
        QModelIndex currentIndex = folderView->rootIndex();
        if (currentIndex.parent().isValid()) {
          QString currentPath = folderModel->filePath(currentIndex);
          QString newPath = folderModel->filePath(currentIndex.parent());

          // 推入当前目录到历史栈
          pushCurrentDirectory(currentPath);

          folderView->setRootIndex(currentIndex.parent());
          updateCurrentFolderLabel(newPath);
        }
      });

  // 打开文件夹
  QObject::connect(pbOpenFolder, &QPushButton::clicked,
                   [this, folderModel, folderView, astPath]() {
                     QString dir = QFileDialog::getExistingDirectory(
                         nullptr, "选择文件夹", astPath);
                     if (!dir.isEmpty()) {
                       QString currentPath =
                           folderModel->filePath(folderView->rootIndex());

                       // 推入当前目录到历史栈
                       pushCurrentDirectory(currentPath);

                       folderView->setRootIndex(folderModel->index(dir));
                       updateCurrentFolderLabel(dir);
                     }
                   });

  // 回退目录
  QObject::connect(pbBackDirectory, &QPushButton::clicked,
                   [this, folderModel, folderView]() {
                     if (!directoryHistory.isEmpty()) {
                       QString previousPath = directoryHistory.pop();
                       folderView->setRootIndex(
                           folderModel->index(previousPath));
                       updateCurrentFolderLabel(previousPath);
                       pbBackDirectory->setEnabled(!directoryHistory.isEmpty());
                     }
                   });

  // 过滤功能
  QObject::connect(
      leFilter, &QLineEdit::textChanged,
      [folderModel, folderView](const QString &text) {
        // 定义支持的文件后缀名
        const QStringList supportedSuffixes = {"*.obj", "*.findvef", "*.off",
                                               "*.fbx", "*.vtk"};

        QStringList filters;
        for (const QString &suffix : supportedSuffixes) {
          filters << (text.isEmpty() ? suffix : "*" + text + suffix);
        }

        folderModel->setNameFilters(filters);
        folderView->setRootIndex(
            folderModel->index(folderView->rootIndex()
                                   .data(QFileSystemModel::FilePathRole)
                                   .toString()));
      });

  folderLayout->addWidget(folderView);

  return folderTab;
}

void FindVEFController::setConnects() {
  // 确保所有控件都已经创建
  if (!cbDrawFlat || !cbDrawFlatWithNormal || !cbDrawModelName ||
      !cbDrawModelFullPath || !cbDrawFileSizeFormat || !cbDrawMeshInfo) {
    return; // 如果控件还没创建，直接返回
  }

  // 添加主tab控件与panel设置同步的信号槽连接
  connect(cbDrawPoints, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawPoints = checked;
    }
  });
  connect(cbDrawWireframe, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawWireframe = checked;
    }
  });
  connect(cbDrawFlat, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawFlat = checked;
    }
  });
  connect(cbDrawFlatWithNormal, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawFlatWithNormal = checked;
    }
  });
  connect(cbDrawNonManifold, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawNonManifold = checked;
    }
  });
  connect(cbDrawBnd, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawBnd = checked;
    }
  });
  connect(cbDrawFindVEF, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawFindVEF = checked;
    }
  });
  connect(cbDrawInfo, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawInfo = checked;
    }
  });
  connect(cbDrawModelName, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawModelName = checked;
    }
  });
  connect(cbDrawModelFullPath, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawModelFullPath = checked;
    }
  });
  connect(cbDrawFileSizeFormat, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawFileSizeFormat = checked;
    }
  });
  connect(cbDrawMeshInfo, &QCheckBox::toggled, [this](bool checked) {
    for (auto &settings : panelSettings) {
      settings.drawMeshInfo = checked;
    }
  });

  // 使用QButtonGroup管理绘制面选项的互斥逻辑

  if (bgDrawFace) {
    QObject::connect(
        bgDrawFace,
        static_cast<void (QButtonGroup::*)(QAbstractButton *, bool)>(
            &QButtonGroup::buttonToggled),
        [this](QAbstractButton *button, bool checked) {
          if (checked) {
            // 当一个按钮被选中时，取消其他按钮的选中状态
            for (QAbstractButton *otherButton : bgDrawFace->buttons()) {
              if (otherButton != button && otherButton->isChecked()) {
                otherButton->setChecked(false);
              }
            }
          }
        });
  }

  // 使用QButtonGroup管理模型信息显示选项的互斥逻辑
  if (bgDrawModelInfo) {
    QObject::connect(
        bgDrawModelInfo,
        static_cast<void (QButtonGroup::*)(QAbstractButton *, bool)>(
            &QButtonGroup::buttonToggled),
        [this](QAbstractButton *button, bool checked) {
          if (checked) {
            // 当一个按钮被选中时，取消其他按钮的选中状态
            for (QAbstractButton *otherButton : bgDrawModelInfo->buttons()) {
              if (otherButton != button && otherButton->isChecked()) {
                otherButton->setChecked(false);
              }
            }
          }
        });
  }
}

// 目录历史管理函数实现
void FindVEFController::pushCurrentDirectory(const QString &currentPath) {
  // 将当前路径推入历史栈（避免重复推入相同路径）
  if (!currentPath.isEmpty() &&
      (directoryHistory.isEmpty() || directoryHistory.top() != currentPath)) {
    directoryHistory.push(currentPath);

    // 限制历史栈大小为20
    if (directoryHistory.size() > 20) {
      directoryHistory.removeFirst();
    }

    // 更新回退按钮状态
    if (pbBackDirectory) {
      pbBackDirectory->setEnabled(true);
    }
  }
}

// 更新当前文件夹标签显示
void FindVEFController::updateCurrentFolderLabel(const QString &path) {
  if (lblCurrentFolder) {
    lblCurrentFolder->setText(path);
    lblCurrentFolder->setToolTip(path); // 工具提示显示完整路径
  }
}

// 设置单个panel的子tab
QWidget *FindVEFController::setupPanelSubTab(int panelId) {
  QWidget *subTab = new QWidget();
  QVBoxLayout *subTabLayout = new QVBoxLayout(subTab);

  // 创建标题标签
  QLabel *panelLabel =
      new QLabel(QString("Panel %1 控制面板").arg(panelId + 1));
  panelLabel->setAlignment(Qt::AlignCenter);
  panelLabel->setStyleSheet(
      "font-size: 14px; font-weight: bold; color: #666; margin-bottom: 10px;");
  subTabLayout->addWidget(panelLabel);

  // 创建几何信息选项组
  QGroupBox *geometryGroup = new QGroupBox("几何信息");
  QVBoxLayout *geometryLayout = new QVBoxLayout(geometryGroup);

  // 为每个panel创建独立的几何绘制选项控件（重命名以避免隐藏成员）
  QCheckBox *panelCbDrawPoints = new QCheckBox("绘制顶点");
  QCheckBox *panelCbDrawWireframe = new QCheckBox("绘制线框");
  QCheckBox *panelCbDrawFlat = new QCheckBox("绘制面");
  QCheckBox *panelCbDrawFlatWithNormal = new QCheckBox("绘制面(法向)");
  QCheckBox *panelCbDrawNonManifold = new QCheckBox("绘制非流形面");
  QCheckBox *panelCbDrawBnd = new QCheckBox("绘制边界");

  QCheckBox *panelCbDrawInfo = new QCheckBox("点/线/面数");

  // 创建FindVEF绘制选项组
  QGroupBox *findvefGroup = new QGroupBox("FindVEF绘制");
  QVBoxLayout *findvefLayout = new QVBoxLayout(findvefGroup);
  QCheckBox *panelCbDrawFindVEF = new QCheckBox("绘制FindVEF");

  // 创建元信息选项组
  QGroupBox *metaGroup = new QGroupBox("元信息");
  QVBoxLayout *metaLayout = new QVBoxLayout(metaGroup);
  QCheckBox *panelCbDrawModelName = new QCheckBox("文件名");
  QCheckBox *panelCbDrawModelFullPath = new QCheckBox("文件完整路径");
  QCheckBox *panelCbDrawFileSizeFormat = new QCheckBox("文件大小/格式");
  QCheckBox *panelCbDrawMeshInfo = new QCheckBox("顶点/面索引");

  // 设置初始值

  PanelDrawSettings &settings = panelSettings[panelId];
  panelCbDrawPoints->setChecked(settings.drawPoints);
  panelCbDrawWireframe->setChecked(settings.drawWireframe);
  panelCbDrawFlat->setChecked(settings.drawFlat);
  panelCbDrawFlatWithNormal->setChecked(settings.drawFlatWithNormal);
  panelCbDrawNonManifold->setChecked(settings.drawNonManifold);
  panelCbDrawBnd->setChecked(settings.drawBnd);

  panelCbDrawFindVEF->setChecked(settings.drawFindVEF);
  panelCbDrawInfo->setChecked(settings.drawInfo);
  panelCbDrawModelName->setChecked(settings.drawModelName);
  panelCbDrawModelFullPath->setChecked(settings.drawModelFullPath);
  panelCbDrawFileSizeFormat->setChecked(settings.drawFileSizeFormat);
  panelCbDrawMeshInfo->setChecked(settings.drawMeshInfo);

  // 创建按钮组管理互斥选项（重命名以避免隐藏成员）

  QButtonGroup *panelBgDrawFace = new QButtonGroup(subTab);
  panelBgDrawFace->addButton(panelCbDrawFlat);
  panelBgDrawFace->addButton(panelCbDrawFlatWithNormal);
  panelBgDrawFace->setExclusive(false);

  QButtonGroup *panelBgDrawModelInfo = new QButtonGroup(subTab);
  panelBgDrawModelInfo->addButton(panelCbDrawModelName);
  panelBgDrawModelInfo->addButton(panelCbDrawModelFullPath);
  panelBgDrawModelInfo->setExclusive(false);

  // 连接信号槽
  connect(panelCbDrawPoints, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawPoints = checked;
          });
  connect(panelCbDrawWireframe, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawWireframe = checked;
          });
  connect(panelCbDrawFlat, &QCheckBox::toggled, [this, panelId](bool checked) {
    panelSettings[panelId].drawFlat = checked;
  });
  connect(panelCbDrawFlatWithNormal, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawFlatWithNormal = checked;
          });
  connect(panelCbDrawNonManifold, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawNonManifold = checked;
          });
  connect(panelCbDrawBnd, &QCheckBox::toggled, [this, panelId](bool checked) {
    panelSettings[panelId].drawBnd = checked;
  });
  connect(panelCbDrawFindVEF, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawFindVEF = checked;
          });
  connect(panelCbDrawInfo, &QCheckBox::toggled, [this, panelId](bool checked) {
    panelSettings[panelId].drawInfo = checked;
  });
  connect(panelCbDrawModelName, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawModelName = checked;
          });
  connect(panelCbDrawModelFullPath, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawModelFullPath = checked;
          });
  connect(panelCbDrawFileSizeFormat, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawFileSizeFormat = checked;
          });
  connect(panelCbDrawMeshInfo, &QCheckBox::toggled,
          [this, panelId](bool checked) {
            panelSettings[panelId].drawMeshInfo = checked;
          });

  // 添加互斥逻辑

  connect(panelBgDrawFace,
          static_cast<void (QButtonGroup::*)(QAbstractButton *, bool)>(
              &QButtonGroup::buttonToggled),
          [this, panelId](QAbstractButton *button, bool checked) {
            if (checked) {
              for (QAbstractButton *otherButton : button->group()->buttons()) {
                if (otherButton != button && otherButton->isChecked()) {
                  otherButton->setChecked(false);
                }
              }
            }
          });

  connect(panelBgDrawModelInfo,
          static_cast<void (QButtonGroup::*)(QAbstractButton *, bool)>(
              &QButtonGroup::buttonToggled),
          [this, panelId](QAbstractButton *button, bool checked) {
            if (checked) {
              for (QAbstractButton *otherButton : button->group()->buttons()) {
                if (otherButton != button && otherButton->isChecked()) {
                  otherButton->setChecked(false);
                }
              }
            }
          });

  // 添加控件到各自的布局
  geometryLayout->addWidget(panelCbDrawPoints);
  geometryLayout->addWidget(panelCbDrawWireframe);
  geometryLayout->addWidget(panelCbDrawFlat);
  geometryLayout->addWidget(panelCbDrawFlatWithNormal);
  geometryLayout->addWidget(panelCbDrawNonManifold);
  geometryLayout->addWidget(panelCbDrawBnd);

  geometryLayout->addWidget(panelCbDrawInfo);

  findvefLayout->addWidget(panelCbDrawFindVEF);

  metaLayout->addWidget(panelCbDrawModelName);
  metaLayout->addWidget(panelCbDrawModelFullPath);
  metaLayout->addWidget(panelCbDrawFileSizeFormat);
  metaLayout->addWidget(panelCbDrawMeshInfo);

  subTabLayout->addWidget(geometryGroup);

  subTabLayout->addWidget(findvefGroup);
  subTabLayout->addWidget(metaGroup);
  subTabLayout->addStretch();

  return subTab;
}

// 更新子tab的显示状态
void FindVEFController::updatePanelTabsVisibility() {
  if (!panelTabWidget)
    return;

  // 获取MainWindow指针
  MainWindow *mainWindow = qobject_cast<MainWindow *>(parent());
  if (!mainWindow)
    return;

  // 统计可见的panel数量
  int visiblePanelCount = 0;
  for (int i = 0; i < NUM_MAX_PANEL; i++) {
    if (mainWindow->wPanel[i] && !mainWindow->wPanel[i]->isHidden()) {
      visiblePanelCount++;
    }
  }

  // 只有当有多个panel可见时才显示子tab
  bool shouldShowTabs = visiblePanelCount > 1;
  panelTabWidget->setVisible(shouldShowTabs);

  // 更新每个子tab的可见性
  for (int i = 0; i < NUM_MAX_PANEL; i++) {
    if (mainWindow->wPanel[i] && !mainWindow->wPanel[i]->isHidden()) {
      panelTabWidget->setTabVisible(i, true);
    } else {
      panelTabWidget->setTabVisible(i, false);
    }
  }
}

// 获取指定panel的绘制参数
const PanelDrawSettings &
FindVEFController::getPanelSettings(int panelId) const {
  if (panelId >= 0 && panelId < NUM_MAX_PANEL) {
    return panelSettings[panelId];
  }
  return panelSettings[0]; // 默认返回第一个panel的设置
}

PanelDrawSettings &FindVEFController::getPanelSettings(int panelId) {
  if (panelId >= 0 && panelId < NUM_MAX_PANEL) {
    return panelSettings[panelId];
  }
  return panelSettings[0]; // 默认返回第一个panel的设置
}
