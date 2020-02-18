#include "viewwidget.h"

ViewWidget::ViewWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  draw_status_ = false;
  shape_ = NULL;
  type_ = Shape::kDefault;

  // 需要设置这个，否则只有鼠标按下才会有 mouseMoveEvent
  this->setMouseTracking(true);
}

ViewWidget::~ViewWidget() {}

void ViewWidget::setLine() { type_ = Shape::kLine; }

void ViewWidget::setRect() { type_ = Shape::kRect; }

void ViewWidget::setFreehand() { type_ = Shape::kFreehand; }

void ViewWidget::setEllipse() { type_ = Shape::kEllipse; }

// 对于多边形，交互逻辑是：
// （按下，抬起）=>鼠标拖动=>（按下抬起）=>（鼠标拖动）=>（右键）
// 这里检测按下即可
void ViewWidget::setPolygon() { type_ = Shape::kPolygon; }

void ViewWidget::mousePressEvent(QMouseEvent *event) {

  if (Qt::RightButton == event->button()) {
    if (type_ == Shape::kPolygon && draw_status_) {
      dynamic_cast<Polygon *>(shape_)->set_finished();
      draw_status_ = false;
      shape_list_.push_back(shape_);
      shape_ = NULL;
    }
  } else if (Qt::LeftButton == event->button()) {
    if (draw_status_) {
	  if (type_ == Shape::kPolygon) {
		// need to anchor a point!
		start_point_ = end_point_ = event->pos();
        shape_->set_start(start_point_);
        shape_->set_end(end_point_);
	  }
    } else {
      switch (type_) {
      case Shape::kLine:
        shape_ = new Line();
        break;
      case Shape::kDefault:
        break;
      case Shape::kRect:
        shape_ = new Rect();
        break;
      case Shape::kFreehand:
        shape_ = new Freehand();
        break;
      case Shape::kEllipse:
        shape_ = new Ellipse();
        break;
      case Shape::kPolygon:
        shape_ = new Polygon();
        break;
      }
      if (shape_ != NULL) {
        draw_status_ = true;
        start_point_ = end_point_ = event->pos();
        shape_->set_start(start_point_);
        shape_->set_end(end_point_);
      }
    }
  }
  update();
}

void ViewWidget::mouseMoveEvent(QMouseEvent *event) {
  if (draw_status_ && shape_ != NULL) {
    end_point_ = event->pos();
    shape_->set_end(end_point_);
  }
}

// Polygon starts drawing on release
void ViewWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (draw_status_ && type_ != Shape::kPolygon) // drawing, so drop
  {
    draw_status_ = false;
    shape_list_.push_back(shape_);
    shape_ = NULL;
  }
}

void ViewWidget::paintEvent(QPaintEvent *) {
  QPainter painter(this);

  for (int i = 0; i < shape_list_.size(); i++) {
    shape_list_[i]->Draw(painter);
  }

  if (shape_ != NULL) {
    shape_->Draw(painter);
  }

  update();
}