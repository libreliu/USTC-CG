#include "viewwidget.h"

ViewWidget::ViewWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);

  this->setMouseTracking(true);
}

ViewWidget::~ViewWidget() {
  
}

void ViewWidget::mousePressEvent(QMouseEvent *event) {
  
  update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event) {

}

// Polygon starts drawing on release
void ViewWidget::mouseReleaseEvent(QMouseEvent *event) {

}

void ViewWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);

  update();
}

void ViewWidget::setMode(mode m)
{
}

void ViewWidget::setType(int type)
{
}

