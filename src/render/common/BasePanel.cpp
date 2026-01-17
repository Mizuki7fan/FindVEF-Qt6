#include "BasePanel.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <iostream>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

BasePanel::BasePanel(QWidget *_parent) : QOpenGLWidget(_parent) { init(); }
BasePanel::~BasePanel() {}

void BasePanel::init(void) {

  // qt stuff
  setAttribute(Qt::WA_NoSystemBackground, true);
  setFocusPolicy(Qt::StrongFocus);
  QSurfaceFormat format;
  format.setSamples(16);
  setFormat(format);
  setAcceptDrops(true); // Enable drag-and-drop events on this widget

  // initialize重置取值
  camera.ModelViewMatrix.fill(0.0);
  camera.ProjectionMatrix.fill(0.0);

  camera.Center = dop::Point3II(0.0, 0.0, 0.0);
  camera.Radius = 0.0;
  camera.TotalTrans = dop::Vec3II(0.0, 0.0, 0.0);

  pro_mode_ = PERSPECTIVE;
  set_pro_mode(PERSPECTIVE);
}

void BasePanel::set_scene_pos(const dop::Point3II &_center, float _radius) {

  camera.Center = _center;
  camera.Radius = _radius;

  update_projection_matrix();
  view_all();
}

// void BasePanel::set_camera(const dop::Point3II&, const dop::Point3II&) {}

void BasePanel::view_all() {

  dop::Vec3II _trans =
      dop::Vec3II(-(camera.ModelViewMatrix[0] * camera.Center[0] +
                    camera.ModelViewMatrix[4] * camera.Center[1] +
                    camera.ModelViewMatrix[8] * camera.Center[2] +
                    camera.ModelViewMatrix[12]),
                  -(camera.ModelViewMatrix[1] * camera.Center[0] +
                    camera.ModelViewMatrix[5] * camera.Center[1] +
                    camera.ModelViewMatrix[9] * camera.Center[2] +
                    camera.ModelViewMatrix[13]),
                  -(camera.ModelViewMatrix[2] * camera.Center[0] +
                    camera.ModelViewMatrix[6] * camera.Center[1] +
                    camera.ModelViewMatrix[10] * camera.Center[2] +
                    camera.ModelViewMatrix[14] + 2.0 * camera.Radius));

  makeCurrent();
  glLoadIdentity();
  glTranslated(_trans[0], _trans[1], _trans[2]);
  glMultMatrixd(&camera.ModelViewMatrix[0]);
  glGetDoublev(GL_MODELVIEW_MATRIX, &camera.ModelViewMatrix[0]);
}

void BasePanel::set_pro_mode(PROJECTION_MODE pm) {

  pro_mode_ = pm;
  update_projection_matrix();
  view_all();
  updateGL();
}

void BasePanel::setDefaultMaterial(void) const {

  // material - 优化材质以配合新的多光源系统
#if 1
  std::vector<GLfloat> matAmbient, matDiffuse, matSpecular;
  GLfloat matShininess;

  // 降低环境光反射，避免过亮
  matAmbient = {0.7f, 0.7f, 0.7f, 1.0f};
  // 适中的漫反射，保持良好的表面细节
  matDiffuse = {0.8f, 0.8f, 0.8f, 1.0f};
  // 降低镜面反射，减少过度高光
  matSpecular = {0.5f, 0.5f, 0.5f, 1.0f};
  // 适中的光泽度，既有质感又不过度反光
  matShininess = 64.0;

  // 备选材质配置（注释保留）
  // GLfloat mat_a[] = { 0.7f, 0.7f, 0.7f, 1.0f };
  // GLfloat mat_d[] = { 0.88f, 0.84f, 0.76f, 1.0f };
  // GLfloat mat_s[] = { 0.4f, 0.4f, 0.4f, 1.0f };
  // GLfloat shine[] = { 120.0f };

  /*GLfloat mat_a[] = { 0.19225f, 0.19225f, 0.19225f, 1.0f };
  GLfloat mat_d[] = { 0.50754f, 0.50754f, 0.50754f, 1.0f };
  GLfloat mat_s[] = { 0.508273f, 0.508273f, 0.508273f, 1.0f };
  GLfloat shine[] = { 51.2f };*/
#else
  // 简化版本 - 蓝色调材质
  GLfloat mat_a[] = {0.0, 0.5, 0.75, 1.0};
  GLfloat mat_d[] = {0.0, 0.5, 1.0, 1.0};
  GLfloat mat_s[] = {0.75, 0.75, 0.75, 1.0};
  GLfloat emission[] = {0.3f, 0.3f, 0.3f, 1.0f};
  GLfloat shine[] = {120.0};
#endif

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient.data());
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse.data());
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular.data());
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);

  // glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_a);
  // glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_d);
  // glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_s);
  // glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
  // glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
}

void BasePanel::setDefaultLight(void) const {

#if 1
  // 设置全局环境光 - 解决大片阴影问题的关键
  GLfloat globalAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); // 双面光照

  // 主光源 - 右上前方 (主要照明)
  GLfloat pos1[] = {10.0f, 10.0f, 10.0f, 0.0f};
  GLfloat col1_diffuse[] = {0.7f, 0.7f, 0.7f, 1.0f};
  GLfloat col1_specular[] = {0.8f, 0.8f, 0.8f, 1.0f};

  // 填充光 - 左下后方 (减少阴影对比度)
  GLfloat pos2[] = {-8.0f, -8.0f, -8.0f, 0.0f};
  GLfloat col2_diffuse[] = {0.4f, 0.4f, 0.4f, 1.0f};
  GLfloat col2_specular[] = {0.2f, 0.2f, 0.2f, 1.0f};

  // 侧光 - 右侧 (增强立体感)
  GLfloat pos3[] = {15.0f, 0.0f, 0.0f, 0.0f};
  GLfloat col3_diffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
  GLfloat col3_specular[] = {0.3f, 0.3f, 0.3f, 1.0f};

  // 底部补光 - 从下方照射 (消除底部阴影)
  GLfloat pos4[] = {0.0f, -12.0f, 5.0f, 0.0f};
  GLfloat col4_diffuse[] = {0.3f, 0.3f, 0.3f, 1.0f};
  GLfloat col4_specular[] = {0.1f, 0.1f, 0.1f, 1.0f};

  // 启用主光源 LIGHT0
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, pos1);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, col1_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, col1_specular);

  // 启用填充光 LIGHT1
  glEnable(GL_LIGHT1);
  glLightfv(GL_LIGHT1, GL_POSITION, pos2);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, col2_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, col2_specular);

  // 启用侧光 LIGHT2
  glEnable(GL_LIGHT2);
  glLightfv(GL_LIGHT2, GL_POSITION, pos3);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, col3_diffuse);
  glLightfv(GL_LIGHT2, GL_SPECULAR, col3_specular);

  // 启用底部补光 LIGHT3
  glEnable(GL_LIGHT3);
  glLightfv(GL_LIGHT3, GL_POSITION, pos4);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, col4_diffuse);
  glLightfv(GL_LIGHT3, GL_SPECULAR, col4_specular);
#else
  // 简化版本 - 只使用环境光和一个主光源
  GLfloat globalAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

  GLfloat pos3[] = {0.0f, 0.0f, 10.0f, 0.0f};
  GLfloat col3[] = {0.8f, 0.8f, 0.8f, 1.0f};
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, pos3);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, col3);
  glLightfv(GL_LIGHT0, GL_SPECULAR, col3);
#endif
}

void BasePanel::mousePressEvent(QMouseEvent *_event) {

  // assert(mouse_mode_ < N_MOUSE_MODES);
  last_point_2D_ = _event->pos();
  last_point_ok_ = map_to_sphere(last_point_2D_, last_point_3D_);
  mouse_mode_ = _event->button();
}

void BasePanel::mouseReleaseEvent(QMouseEvent *) {

  // assert(mouse_mode_ < N_MOUSE_MODES);
  mouse_mode_ = Qt::NoButton;
  last_point_ok_ = false;
}

void BasePanel::mouseMoveEvent(QMouseEvent *_event) {

  QPoint newPoint2D = _event->pos();

  // enable GL context
  makeCurrent();

  if (last_point_ok_) {
    //	translation(newPoint2D);
    switch (mouse_mode_) {
    case Qt::LeftButton:
      rotation(newPoint2D);
      break;
    case Qt::RightButton:
      translation(newPoint2D);
      break;
    default:
      break;
    }
  } // end of if

  // remember this point
  last_point_2D_ = newPoint2D;
  last_point_ok_ = map_to_sphere(last_point_2D_, last_point_3D_);

  // trigger redraw
  updateGL();
}

void BasePanel::wheelEvent(QWheelEvent *_event) {

  // Use the mouse wheel to zoom in/out
  wheel_var = -static_cast<double>(_event->angleDelta().y()) / 120.0 * 0.05 *
              camera.Radius;
  translate(dop::Vec3II(0.0, 0.0, static_cast<float>(wheel_var)));
  updateGL();
  _event->accept();
}

// 拖入数据类型的检查, 由于下层有判断文件的后缀名的步骤, 本步骤可以删除
void BasePanel::dragEnterEvent(QDragEnterEvent *_event) {

  if (_event->mimeData()->hasFormat("text/uri-list")) {
    _event->acceptProposedAction();
  }
}

void BasePanel::initializeGL() {

  auto a = context();
  auto b = a->functions();
  // OpenGL state
  b->glClearColor(1.0, 1.0, 1.0, 1.0);
  b->glEnable(GL_DEPTH_TEST);
  b->glEnable(GL_MULTISAMPLE);
  b->glEnable(GL_LINE_SMOOTH);
  b->glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  b->glEnable(GL_POINT_SMOOTH);
  b->glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  b->glEnable(GL_BLEND);
  b->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Material
  setDefaultMaterial();
  // 启用颜色材质 - 关键设置！允许glColor3f与光照计算结合
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  // Lighting
  glLoadIdentity();
  setDefaultLight();

  // scene pos and size
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  // glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix_);

  // for initialize all the viewports
  glGetDoublev(GL_MODELVIEW_MATRIX, &camera.ModelViewMatrix[0]);

  set_scene_pos(dop::Point3II(0.0, 0.0, 0.0), 1.0);
}

void BasePanel::resizeGL(int w, int h) {

  // https://learnopengl-cn.github.io/01%20Getting%20started/03%20Hello%20Window/
  glViewport(0, 0, (GLint)w, (GLint)h);
  update_projection_matrix();
  updateGL();
}

void BasePanel::paintGL() {

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  update_projection_matrix();
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(&camera.ModelViewMatrix[0]);
  draw();
}

bool BasePanel::map_to_sphere(const QPoint &_v2D, dop::Point3II &_v3D) {

  // This is actually doing the Sphere/Hyperbolic sheet hybrid thing,
  // based on Ken Shoemake's ArcBall in Graphics Gems IV, 1993.
  double x = (2.0 * _v2D.x() - width()) / width();
  double y = -(2.0 * _v2D.y() - height()) / height();
  double xval = x;
  double yval = y;
  double x2y2 = xval * xval + yval * yval;

  const double rsqr = kTrackBallRadius * kTrackBallRadius;
  _v3D = dop::Point3II(xval, yval, 0.0);

  if (x2y2 < 0.5 * rsqr) {
    _v3D = dop::Point3II(xval, yval, std::sqrt(rsqr - x2y2));
  } else {
    _v3D = dop::Point3II(xval, yval, 0.5 * rsqr / std::sqrt(x2y2));
  }

  return true;
}

void BasePanel::update_projection_matrix() {

  // makeCurrent();
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // std::cout << "("<<width() << " " << height()<<")" << std::endl;
  if (PERSPECTIVE == pro_mode_) {
    // gluPerspective(45.0, (GLfloat)width() / (GLfloat)height(), 0.01 * Radius,
    // 100.0 * Radius);
    glFrustum(-0.01 * camera.Radius * (sqrt(2.0) - 1) * width() / height(),
              0.01 * camera.Radius * (sqrt(2.0) - 1) * width() / height(),
              -0.01 * camera.Radius * (sqrt(2.0) - 1),
              0.01 * camera.Radius * (sqrt(2.0) - 1), 0.01 * camera.Radius,
              100.0 * camera.Radius);
  } else {
  }

  glGetDoublev(GL_PROJECTION_MATRIX, &camera.ProjectionMatrix[0]);
  glMatrixMode(GL_MODELVIEW);
}

void BasePanel::translate(const dop::Vec3II &_trans) {

  // Translate the object by _trans
  // Update modelview_matrix_
  makeCurrent();
  glLoadIdentity();
  glTranslated(_trans[0], _trans[1], _trans[2]);
  glMultMatrixd(&camera.ModelViewMatrix[0]);
  glGetDoublev(GL_MODELVIEW_MATRIX, &camera.ModelViewMatrix[0]);
  // CGAL向量不支持直接+=操作，需要创建新向量
  camera.TotalTrans = camera.TotalTrans + _trans;
}

void BasePanel::rotate(const dop::Vec3II &_axis, double _angle) {

  // Rotate around center center_, axis _axis, by angle _angle
  // Update modelview_matrix_

  dop::Vec3II t(camera.ModelViewMatrix[0] * camera.Center[0] +
                    camera.ModelViewMatrix[4] * camera.Center[1] +
                    camera.ModelViewMatrix[8] * camera.Center[2] +
                    camera.ModelViewMatrix[12],
                camera.ModelViewMatrix[1] * camera.Center[0] +
                    camera.ModelViewMatrix[5] * camera.Center[1] +
                    camera.ModelViewMatrix[9] * camera.Center[2] +
                    camera.ModelViewMatrix[13],
                camera.ModelViewMatrix[2] * camera.Center[0] +
                    camera.ModelViewMatrix[6] * camera.Center[1] +
                    camera.ModelViewMatrix[10] * camera.Center[2] +
                    camera.ModelViewMatrix[14]);

  makeCurrent();
  glLoadIdentity();
  glTranslatef(t[0], t[1], t[2]);
  glRotated(_angle, _axis[0], _axis[1], _axis[2]);
  glTranslatef(-t[0], -t[1], -t[2]);
  glMultMatrixd(&camera.ModelViewMatrix[0]);
  glGetDoublev(GL_MODELVIEW_MATRIX, &camera.ModelViewMatrix[0]);
}

void BasePanel::translation(QPoint p) {

  double z = -(camera.ModelViewMatrix[2] * camera.Center[0] +
               camera.ModelViewMatrix[6] * camera.Center[1] +
               camera.ModelViewMatrix[10] * camera.Center[2] +
               camera.ModelViewMatrix[14]) /
             (camera.ModelViewMatrix[3] * camera.Center[0] +
              camera.ModelViewMatrix[7] * camera.Center[1] +
              camera.ModelViewMatrix[11] * camera.Center[2] +
              camera.ModelViewMatrix[15]);

  double w = width();
  double h = height();
  double aspect = w / h;
  double near_plane = 0.01 * camera.Radius;
  double top = tan(fovy() / 2.0f * M_PI / 180.0f) * near_plane;
  double right = aspect * top;

  double dx = p.x() - last_point_2D_.x();
  double dy = p.y() - last_point_2D_.y();

  translate(dop::Vec3II(2.0 * dx / w * right / near_plane * z,
                        -2.0 * dy / h * top / near_plane * z, 0.0f));
}

void BasePanel::rotation(QPoint p) {

  dop::Point3II newPoint3D;
  bool newPoint_hitSphere = map_to_sphere(p, newPoint3D);
  if (newPoint_hitSphere) {
    dop::Vec3II axis = CGAL::cross_product(
        dop::Vec3II(last_point_3D_.x(), last_point_3D_.y(), last_point_3D_.z()),
        dop::Vec3II(newPoint3D.x(), newPoint3D.y(), newPoint3D.z()));
    double axis_sqrnorm = axis.squared_length();
    if (axis_sqrnorm < 1e-7) {
      axis = dop::Vec3II(1, 0, 0);
    } else {
      axis /= std::sqrt(axis_sqrnorm);
      // axis.normalize();
    }
    // find the amount of rotation
    dop::Vec3II d = last_point_3D_ - newPoint3D;
    double norm = std::sqrt(d.squared_length());
    double t = 0.5 * norm / kTrackBallRadius;
    if (t < -1.0)
      t = -1.0;
    else if (t > 1.0)
      t = 1.0;
    double phi = 2.0 * asin(t);
    double angle = phi * 180.0 / M_PI;
    rotate(axis, angle);
  }
}
