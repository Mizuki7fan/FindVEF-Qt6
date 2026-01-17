#pragma once
#include "../common/BasePanel.h"
#include "OpenGL3Renderer.h"
#include <QOpenGLWidget>

class OpenGL3FindVEFPanel : public BasePanel {

  Q_OBJECT

public:
  explicit OpenGL3FindVEFPanel(QWidget *parent = nullptr);
  ~OpenGL3FindVEFPanel() override;

protected:
  // QOpenGLWidget overrides
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;

  // Custom rendering methods
  void renderScene();

  // Event handling
  void dropEvent(QDropEvent *event) override;

private:
  OpenGL3Renderer *m_renderer;
};