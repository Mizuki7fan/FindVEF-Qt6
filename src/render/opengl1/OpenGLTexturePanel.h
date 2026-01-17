#pragma once
#include "OpenGLFindVEFPanel.h"

// 纹理面板：仅支持平移与缩放，不支持旋转
class OpenGLTexturePanel : public OpenGLFindVEFPanel {
public:
  OpenGLTexturePanel(MainWindow *parent, int panel_id)
      : OpenGLFindVEFPanel(parent, panel_id) {}
  ~OpenGLTexturePanel() override {}

  void mousePressEvent(QMouseEvent *event) override {
    // 屏蔽左键按下（防止进入旋转模式），允许右键/其他用于平移
    if (event->button() == Qt::LeftButton) {
      event->accept();
      return;
    }
    OpenGLFindVEFPanel::mousePressEvent(event);
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    // 屏蔽左键拖动（不旋转），保留右键平移
    if (event->buttons() & Qt::LeftButton) {
      event->accept();
      return;
    }
    OpenGLFindVEFPanel::mouseMoveEvent(event);
  }

  void wheelEvent(QWheelEvent *event) override {
    // 保留滚轮缩放
    OpenGLFindVEFPanel::wheelEvent(event);
  }
};