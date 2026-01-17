#include "Controller.h"

Controller::Controller(QWidget* _parent) : QWidget(_parent) {
  setWindowFlags(windowFlags() & ~Qt::WindowMinMaxButtonsHint &
                 ~Qt::WindowCloseButtonHint);
}

Controller::~Controller() {}

void Controller::Do() {
  saWnd = new QScrollArea();
  saWnd->setFocusPolicy(Qt::NoFocus);
  saWnd->setFrameStyle(QFrame::NoFrame);
  saWnd->setWidgetResizable(true);
  saWnd->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  globalLayout = new QVBoxLayout();
  setGlobalLayout();
  setConnects();
  globalLayout->setContentsMargins(0, 0, 0, 0);

  QWidget* wWnd = new QWidget();
  wWnd->setLayout(globalLayout);
  saWnd->setWidget(wWnd);
  QVBoxLayout* layout = new QVBoxLayout();
  layout->addWidget(saWnd);
  this->setLayout(layout);
}
