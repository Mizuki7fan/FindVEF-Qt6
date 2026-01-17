#pragma once
#include <QScrollArea>
#include <QWidget>
#include <QtGui>
#include <QtWidgets>

class Controller : public QWidget {
  Q_OBJECT
 public:
  Controller(QWidget* _parent = 0);
  ~Controller();

 public:
  virtual void setGlobalLayout() = 0;
  virtual void setConnects() = 0;
  void Do();

 public:
  QVBoxLayout* globalLayout;
  QScrollArea* saWnd;
};

// 自定义的Seperator格式
class CustomSeperator : public QLabel {
 public:
  CustomSeperator(const QString& text, QWidget* parent = nullptr)
      : QLabel(parent) {
    QString format = QString("====  %1  ====").arg(text);
    this->setText(format);
    this->setAlignment(Qt::AlignCenter);
    // 设置样式表（可选）
    this->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
    QFont font = this->font();
    font.setPointSize(10);
    font.setFamily("Arial");
    this->setFont(font);
  }
};
