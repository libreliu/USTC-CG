#include "viewwidget.h"
#include "minidraw.h"
#include <exception>

ViewWidget::ViewWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);

  this->parent_in_law = dynamic_cast<MiniDraw *>(this->parent());

  this->next_mode = this->cur_mode = ViewWidget::mode::VIEW;
  this->next_shape = this->cur_shape =
      DrawContext::ShapeManager::ShapeType::Ellipse;
  shape_working = nullptr;
  shape_working_points = 0;

  this->setMouseTracking(true);
}

ViewWidget::~ViewWidget() {
  if (shape_working) {
   delete shape_working;
  }
  for (auto &p : shape_list) {
   delete p;
  }
}

void ViewWidget::doModeSwitch() {
  cur_mode = next_mode;
  parent_in_law->setModeStatus(cur_mode);
}

void ViewWidget::pushShape() {
  shape_list.push_back(shape_working);
  shape_working = nullptr;
  shape_working_points = 0;
}

void ViewWidget::clearShape() {
  if (shape_working)
    delete shape_working;
  shape_working = nullptr;
  shape_working_points = 0;
}

void ViewWidget::sendWarning(const QString s) { parent_in_law->putMessage(s); }

void ViewWidget::mousePressEvent(QMouseEvent *event) {
  switch (cur_mode) {
  case VIEW:
    // do nothing
    break;
  case ADD_POINT:
    if (event->button() == Qt::LeftButton) {
      if (!shape_working) { // Add a shape
        cur_shape = next_shape;
        parent_in_law->setShapeStatus(
            (DrawContext::ShapeManager::ShapeType)cur_shape);

        shape_working = DrawContext::ShapeManager::getFactory(cur_shape)(0);
        if (!shape_working) {
          sendWarning(tr("Error: Can't construct the shape chosen."));
          return;
        }
        // check for validity
        if (shape_working->getAttr("raw-mode") == 1) {
          sendWarning(tr("Error: This shape can only be used in RAW mode."));
          clearShape();
          doModeSwitch();
          return;
        }

        // add two point, the latter used for "roaming"
        shape_working->addCtrlPoint(event->pos());
        shape_working->addCtrlPoint(event->pos());
        shape_working_points = 2;
      } else {
        int max_points = shape_working->getAttr("point-required");
        if (max_points >= 0 && max_points == shape_working_points) {
          // just fix this point and go idle
          shape_working->modifyLastCtrlPoint(event->pos());
          shape_working->setAttr("finished", 1);
          pushShape();
          doModeSwitch();
        } else {
          // go add another one
          shape_working->modifyLastCtrlPoint(event->pos());
          shape_working->addCtrlPoint(event->pos());
          shape_working_points++;
        }
      }
    } else if (event->button() == Qt::RightButton) { // cut a shape
      if (shape_working) {
        int max_points = shape_working->getAttr("point-required");
        if (max_points >= 0 &&
            max_points < shape_working_points) { // no sufficient point
          sendWarning(tr("Warning: no sufficient point(s), ignored"));
        } else { // either no limit on point-required, or point already
                 // sufficient
          // just fix this point and go idle
          shape_working->modifyLastCtrlPoint(event->pos());
          shape_working->setAttr("finished", 1);
          pushShape();
          doModeSwitch();
        }
      }
    }

    break;
  case RAW:
    // only left click is detected
    // record things while moving
    if (event->button() == Qt::LeftButton) {
      if (!shape_working) { // Add a shape
        cur_shape = next_shape;
        parent_in_law->setShapeStatus(
            (DrawContext::ShapeManager::ShapeType)cur_shape);

        shape_working = DrawContext::ShapeManager::getFactory(cur_shape)(0);
        if (!shape_working) {
          sendWarning(tr("Error: Can't construct the shape chosen."));
          return;
        }
        // check for validity
        if (shape_working->getAttr("raw-mode") != 1) {
          sendWarning(tr("Error: This shape can't be used in RAW mode."));
          clearShape();
          doModeSwitch();
          return;
        }

        // add the initial point
        shape_working->addCtrlPoint(event->pos());
      }
    }
    break;
  }

  update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event) {
  switch (cur_mode) {
  case ADD_POINT:
    if (shape_working) {
      shape_working->modifyLastCtrlPoint(event->pos());
    }
    break;
  case RAW:
    if (shape_working) {
      if (shape_working->getLastCtrlPoint() != event->pos()) {
        shape_working->addCtrlPoint(event->pos());
      }
    }
  }

  parent_in_law->setCursorStatus(event->pos().x(), event->pos().y());
}

// Polygon starts drawing on release
void ViewWidget::mouseReleaseEvent(QMouseEvent *event) {
  switch (cur_mode) {
  case RAW:
    if (shape_working) { // save the things up
      if (shape_working->getLastCtrlPoint() != event->pos()) {
        shape_working->addCtrlPoint(event->pos());
      }
      pushShape();
      doModeSwitch();
    }
    break;
  }
}

void ViewWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);

  for (auto &p : shape_list) {
    if (cur_mode != VIEW)
      p->DrawCtrlPoints(painter);
    p->Draw(painter);
  }

  if (shape_working) {
    if (cur_mode != VIEW)
      shape_working->DrawCtrlPoints(painter);
    shape_working->Draw(painter);
  }

  update();
}

int ViewWidget::getCurShape() { return this->cur_shape; }

void ViewWidget::setMode(mode m) {
  if (!shape_working) {
    next_mode = cur_mode = m;
    parent_in_law->setModeStatus(m);
  } else {
    next_mode = m;
    sendWarning(tr("Notice: Next mode has changed."));
  }
}

void ViewWidget::setShape(int s) {
  if (!shape_working) {
    next_shape = cur_shape = s;
    parent_in_law->setShapeStatus((DrawContext::ShapeManager::ShapeType)s);
  } else {
    next_shape = s;
    sendWarning(tr("Notice: Next shape has changed."));
  }
}
