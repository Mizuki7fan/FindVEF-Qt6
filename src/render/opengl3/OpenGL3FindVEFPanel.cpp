#include "OpenGL3FindVEFPanel.h"
#include <QDropEvent>
#include <QMimeData>
#include <QOpenGLContext>
#include <QUrl>
#include <iostream>

OpenGL3FindVEFPanel::OpenGL3FindVEFPanel(QWidget *parent)
    : BasePanel(parent), m_renderer(nullptr) {

  // 设置OpenGL格式
  QSurfaceFormat format;
  format.setVersion(3, 0);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setSamples(4); // 4x MSAA
  setFormat(format);
}

OpenGL3FindVEFPanel::~OpenGL3FindVEFPanel() {
  makeCurrent();
  delete m_renderer;
  doneCurrent();
}

void OpenGL3FindVEFPanel::initializeGL() {
  try {
    m_renderer = new OpenGL3Renderer();
    m_renderer->initialize();

    // 设置背景色
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  } catch (const std::exception &e) {
    qWarning("Failed to initialize OpenGL 3.0 renderer: %s", e.what());
  }
}

void OpenGL3FindVEFPanel::paintGL() {
  if (!m_renderer)
    return;

  m_renderer->clear();
  m_renderer->beginRender();

  renderScene();

  m_renderer->endRender();
}

void OpenGL3FindVEFPanel::resizeGL(int width, int height) {
  if (m_renderer) {
    m_renderer->resize(width, height);
  }
}

void OpenGL3FindVEFPanel::renderScene() {
  if (!m_renderer)
    return;

  // 这里可以添加OpenGL 3.0特定的渲染逻辑
  // 例如使用着色器渲染网格、点、线等

  // TODO: 实现OpenGL 3.0的场景渲染逻辑
}

void OpenGL3FindVEFPanel::dropEvent(QDropEvent *event) {
  // OpenGL3FindVEFPanel 的拖拽事件处理
  // 目前提供基本的实现，可以根据需要扩展

  if (event->mimeData()->hasUrls()) {
    QList<QUrl> urls = event->mimeData()->urls();
    for (const QUrl &url : urls) {
      QString filePath = url.toLocalFile();
      std::cout << "OpenGL3FindVEFPanel: Dropped file: "
                << filePath.toStdString() << std::endl;

      // TODO: 根据需要添加文件处理逻辑
      // 例如加载模型文件等
    }
    event->acceptProposedAction();
  } else {
    event->ignore();
  }
}
