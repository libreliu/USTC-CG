#include "viewwidget.h"

ViewWidget::ViewWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  draw_status_ = false;
  shape_ = NULL;
  type_ = shape::kDefault;

  // 需要设置这个，否则只有鼠标按下才会有 mouseMoveEvent
  this->setMouseTracking(true);
}

ViewWidget::~ViewWidget() {}

void ViewWidget::setLine() { type_ = shape::kLine; }

void ViewWidget::setRect() { type_ = shape::kRect; }

void ViewWidget::setFreehand() { type_ = shape::kFreehand; }

void ViewWidget::setEllipse() { type_ = shape::kEllipse; }

// 对于多边形，交互逻辑是：
// （按下，抬起）=>鼠标拖动=>（按下抬起）=>（鼠标拖动）=>（右键）
// 这里检测按下即可
void ViewWidget::setPolygon() { type_ = shape::kPolygon; }

void ViewWidget::mousePressEvent(QMouseEvent *event) {

  if (Qt::RightButton == event->button()) {
    if (type_ == shape::kPolygon && draw_status_) {
      dynamic_cast<Polygon *>(shape_)->set_finished();
      draw_status_ = false;
      shape_list_.push_back(shape_);
      shape_ = NULL;
    }
  } else if (Qt::LeftButton == event->button()) {
    if (draw_status_) {
	  if (type_ == shape::kPolygon) {
		// need to anchor a point!
		start_point_ = end_point_ = event->pos();
        shape_->set_start(start_point_);
        shape_->set_end(end_point_);
	  }
    } else {
      switch (type_) {
      case shape::kLine:
        shape_ = new Line();
        break;
      case shape::kDefault:
        break;
      case shape::kRect:
        shape_ = new Rect();
        break;
      case shape::kFreehand:
        shape_ = new Freehand();
        break;
      case shape::kEllipse:
        shape_ = new Ellipse();
        break;
      case shape::kPolygon:
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
  if (draw_status_ && type_ != shape::kPolygon) // drawing, so drop
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