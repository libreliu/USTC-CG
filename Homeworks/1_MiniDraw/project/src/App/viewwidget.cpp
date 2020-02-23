#include "viewwidget.h"
#include <exception>
#include "minidraw.h"

ViewWidget::ViewWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);

  this->cur_mode = ViewWidget::mode::VIEW;
  this->cur_shape = DrawContext::ShapeManager::ShapeType::Ellipse;
  this->shape_working = nullptr;
  this->shape_working_points = 0;

  this->setMouseTracking(true);
}

ViewWidget::~ViewWidget() {
  if (shape_working) {
    delete shape_working;
  }
  for (auto &p: shape_list) {
    delete p;
  }
}

void ViewWidget::mousePressEvent(QMouseEvent *event) {
  switch (cur_mode) {
    case VIEW:
      // do nothing
      break;
    case ADD_POINT:
      if (event->button() == Qt::LeftButton) {
        if (!shape_working) { // Add a shape
          shape_working = DrawContext::ShapeManager::getFactory(cur_shape)(0);
          if (!shape_working) {
            fprintf(stderr, "ERROR on mouse press\n");
            return;
          }
          shape_working->addCtrlPoint(event->pos());
          shape_working_points = 1;
        } else {
          shape_working->addCtrlPoint(event->pos());
          shape_working_points++;
        }
      } else if (event->button() == Qt::RightButton) { // cut a shape
        int max_points = shape_working->getAttr("point-required");
        if (max_points >= 0 && max_points > shape_working_points) {
          // send warning, todo
        } else {
          shape_list.push_back(shape_working);
          shape_working = nullptr;
          shape_working_points = 0;
        }
      }

      if (shape_working) {  // check if we've had enough points for the shape
        int max_points = shape_working->getAttr("point-required");
        if (max_points >= 0 && max_points == shape_working_points) {
          shape_list.push_back(shape_working);
          shape_working = nullptr;
          shape_working_points = 0;
        }
      }

      break;
  }


  update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event) {
  if (shape_working) {
    shape_working->modifyLastCtrlPoint(event->pos());
  }
  MiniDraw* p = dynamic_cast<MiniDraw *>(this->parent());
  p->setCursorStatus(event->pos().x(), event->pos().y());
}

// Polygon starts drawing on release
void ViewWidget::mouseReleaseEvent(QMouseEvent *event) {

}

void ViewWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);

  for (auto &p: shape_list) {
    p->DrawCtrlPoints(painter);
    p->Draw(painter);
  }

  if (shape_working) {
    shape_working->DrawCtrlPoints(painter);
    shape_working->Draw(painter);
  }

  update();
}

int ViewWidget::getCurShape() {
  return this->cur_shape;
}

void ViewWidget::setMode(mode m)
{
  this->cur_mode = m;
  MiniDraw* p = dynamic_cast<MiniDraw *>(this->parent());
  p->setModeStatus(m);
}

void ViewWidget::setShape(int s)
{
  this->cur_shape = s;
  MiniDraw* p = dynamic_cast<MiniDraw *>(this->parent());
  p->setShapeStatus((DrawContext::ShapeManager::ShapeType)s);
}

