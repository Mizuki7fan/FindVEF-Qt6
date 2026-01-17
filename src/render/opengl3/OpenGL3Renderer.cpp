#include "OpenGL3Renderer.h"
#include "../../core/mesh/MeshData.h"
#include "../../utils/FindVEFHandler.h"
#include <QMatrix4x4>
#include <stdexcept>

OpenGL3Renderer::OpenGL3Renderer()
    : m_shaderProgram(nullptr), m_width(800), m_height(600),
      m_lightingMode(LightingMode::Flat), m_currentMaterial(0) {}

OpenGL3Renderer::~OpenGL3Renderer() { cleanup(); }

void OpenGL3Renderer::initialize() {
  if (!initializeOpenGLFunctions()) {
    throw std::runtime_error("Failed to initialize OpenGL 3.0 functions");
  }

  setupShaders();
  setupBuffers();

  // 启用深度测试
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // 启用面剔除
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
}

void OpenGL3Renderer::cleanup() {
  if (m_shaderProgram) {
    delete m_shaderProgram;
    m_shaderProgram = nullptr;
  }

  m_vao.destroy();
  m_vertexBuffer.destroy();
  m_indexBuffer.destroy();
}

void OpenGL3Renderer::setupShaders() {
  m_shaderProgram = new QOpenGLShaderProgram();

  // 顶点着色器
  const char *vertexShaderSource = R"(
        #version 130
        in vec3 position;
        in vec3 normal;
        in vec3 color;
        
        uniform mat4 mvpMatrix;
        uniform mat4 modelViewMatrix;
        uniform mat3 normalMatrix;
        
        out vec3 fragColor;
        out vec3 fragNormal;
        out vec3 fragPosition;
        
        void main() {
            gl_Position = mvpMatrix * vec4(position, 1.0);
            fragColor = color;
            fragNormal = normalize(normalMatrix * normal);
            fragPosition = vec3(modelViewMatrix * vec4(position, 1.0));
        }
    )";

  // 片段着色器
  const char *fragmentShaderSource = R"(
        #version 130
        in vec3 fragColor;
        in vec3 fragNormal;
        in vec3 fragPosition;
        
        uniform int lightingMode;
        uniform vec3 lightPosition;
        uniform vec3 lightColor;
        uniform vec3 ambientColor;
        
        out vec4 finalColor;
        
        void main() {
            if (lightingMode == 0) {
                // 无光照模式
                finalColor = vec4(fragColor, 1.0);
            } else {
                // 简单光照计算
                vec3 lightDir = normalize(lightPosition - fragPosition);
                float diff = max(dot(fragNormal, lightDir), 0.0);
                vec3 diffuse = diff * lightColor;
                vec3 ambient = ambientColor;
                vec3 result = (ambient + diffuse) * fragColor;
                finalColor = vec4(result, 1.0);
            }
        }
    )";

  if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex,
                                                vertexShaderSource)) {
    throw std::runtime_error("Failed to compile vertex shader");
  }

  if (!m_shaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment,
                                                fragmentShaderSource)) {
    throw std::runtime_error("Failed to compile fragment shader");
  }

  if (!m_shaderProgram->link()) {
    throw std::runtime_error("Failed to link shader program");
  }
}

void OpenGL3Renderer::setupBuffers() {
  m_vao.create();
  m_vao.bind();

  m_vertexBuffer.create();
  m_vertexBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

  m_indexBuffer.create();
  m_indexBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

  m_vao.release();
}

void OpenGL3Renderer::resize(int width, int height) {
  m_width = width;
  m_height = height;
  glViewport(0, 0, width, height);
}

void OpenGL3Renderer::setScenePosition(const dop::Point3II &center,
                                       float radius) {
  // TODO: 实现场景位置设置
}

void OpenGL3Renderer::updateProjectionMatrix() {
  // TODO: 实现投影矩阵更新
}

void OpenGL3Renderer::updateModelViewMatrix() {
  // TODO: 实现模型视图矩阵更新
}

void OpenGL3Renderer::translate(const dop::Vec3II &trans) {
  // TODO: 实现平移变换
}

void OpenGL3Renderer::rotate(const dop::Vec3II &axis, double angle) {
  // TODO: 实现旋转变换
}

void OpenGL3Renderer::beginRender() {
  m_shaderProgram->bind();
  m_vao.bind();
}

void OpenGL3Renderer::endRender() {
  m_vao.release();
  m_shaderProgram->release();
}

void OpenGL3Renderer::clear() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGL3Renderer::drawMesh(const dop::SM_with_Meta &mesh) {
  // TODO: 实现网格绘制
}

void OpenGL3Renderer::drawPoints() {
  // TODO: 实现点绘制
}

void OpenGL3Renderer::drawWireframe() {
  // TODO: 实现线框绘制
}

void OpenGL3Renderer::drawFlat() {
  // TODO: 实现平面绘制
}

void OpenGL3Renderer::drawFlatWithNormal() {
  // TODO: 实现带法线的平面绘制
}

void OpenGL3Renderer::drawFindVEF(const FindVEFHandler &handler) {
  // TODO: 实现FindVEF绘制
}

void OpenGL3Renderer::setLightingMode(LightingMode mode) {
  m_lightingMode = mode;
  if (m_shaderProgram) {
    m_shaderProgram->bind();
    m_shaderProgram->setUniformValue("lightingMode", static_cast<int>(mode));
    m_shaderProgram->release();
  }
}

void OpenGL3Renderer::setMaterial(int materialId) {
  m_currentMaterial = materialId;
}