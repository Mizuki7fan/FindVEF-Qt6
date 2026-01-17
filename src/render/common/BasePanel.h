#pragma once
#include <QOpenGLWidget>

#include "../../core/geometry/surfacemesh.h"

class QOpenGLTexture;

constexpr double kTrackBallRadius = 0.6;

class BasePanel : public QOpenGLWidget {
  Q_OBJECT
public:
  BasePanel(QWidget *_parent = 0);

  // Destructor.
  virtual ~BasePanel();
  void updateGL() { update(); }

private:
  void init(void);

public:
  /* Sets the center and size of the whole scene.
     The _center is used as fixpoint for rotations and for adjusting
     the camera/viewer (see view_all()). */
  void set_scene_pos(const dop::Point3II &_center, float _radius);
  // void set_camera(const dop::Point3II& _center_pos,
  //                 const dop::Point3II& _target_pos);

  void view_all();
  float radius() const { return camera.Radius; }
  const dop::Point3II &center() const { return camera.Center; }
  const GLdouble *modelview_matrix() const {
    return &camera.ModelViewMatrix[0];
  }

  enum PROJECTION_MODE {
    PERSPECTIVE = 0,
    //		ORTHOTROPIC2D,
  };
  enum OP_MODE {
    TRANSITION = 0,
  };
  const GLdouble *projection_matrix() const {
    return &camera.ProjectionMatrix[0];
  }
  float fovy() const { return 45.0f; }
  void set_pro_mode(PROJECTION_MODE pm);
  int pro_mode() const { return pro_mode_; }
  //	void glPrint(const char* pstr);

protected:
  int material_id = 0;
  int n_material = 0;

  // Qt mouse events
  enum MaterialType {
    MaterialDefault,
    MaterialGold,
    MaterialSilver,
    MaterialEmerald,
    MaterialTin,
    MaterialBrass,
    MaterialBronze,
    MaterialBrightBronze,
    MaterialChromium,
    MaterialCopper,
    MaterialBrightCopper,
    MaterialGold2,
    MaterialBrightGold,
    MaterialWax,
    MaterialSliver2,
    MaterialBrightSliver2,
    MaterialEmerald2,
    MaterialJade,
    MaterialObsidian,
    MaterialPearl,
    MaterialRuby,
    MaterialBeryl,
    MaterialPlastics,
    MaterialRubber,
    MaterialViolet,
    MaterialBlue,
    MaterialDIY
  };
  void setDefaultMaterial(void) const;
  void setDefaultLight(void) const;

  virtual void mousePressEvent(QMouseEvent *) = 0;
  virtual void mouseReleaseEvent(QMouseEvent *) = 0;
  virtual void mouseMoveEvent(QMouseEvent *) = 0;
  virtual void wheelEvent(QWheelEvent *) = 0;
  virtual void dragEnterEvent(QDragEnterEvent *) = 0;
  virtual void dropEvent(QDropEvent *) = 0;

  void update_projection_matrix();
  // translate the scene and update modelview matrix
  void translate(const dop::Vec3II &_trans);
  // rotate the scene (around its center) and update modelview matrix
  void rotate(const dop::Vec3II &_axis, double _angle);

  void translation(QPoint p);
  void rotation(QPoint p);
  PROJECTION_MODE pro_mode_;
  double w_left;
  double w_right;
  double w_top;
  double w_bottom;
  Qt::MouseButton mouse_mode_;
  OP_MODE op_mode_;

public:
  double wheel_var = 0;
  struct CameraConfiguration {
    dop::Point3II Center;
    double Radius;
    dop::Vec3II TotalTrans = dop::Vec3II(0, 0, 0);
    std::array<double, 16> ProjectionMatrix;
    std::array<double, 16> ModelViewMatrix;
  };
  CameraConfiguration camera;

protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  virtual void draw() = 0;
  // void drawAxis();//不启用, 如果需要找实现可以翻其他仓库

protected:
  // virtual trackball: map 2D screen point to unit sphere
  bool map_to_sphere(const QPoint &_point, dop::Point3II &_result);

  QPoint last_point_2D_;
  dop::Point3II last_point_3D_;
  bool last_point_ok_;
};
