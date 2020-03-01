#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets>
#include <iostream>

#include "IDWWarper.h"
#include "RBFWarper.h"


using std::cout;
using std::endl;

ImageWidget::ImageWidget(void) {
  ptr_image_ = new QImage();
  ptr_image_backup_ = new QImage();

  setMouseTracking(true);
  mouse_status_ = false;
}

ImageWidget::~ImageWidget(void) {
    delete ptr_image_;
    delete ptr_image_backup_;
}

void ImageWidget::loadDebug(void) {
    point_.clear();
    point_.push_back(std::make_pair(IntPoint(0, 0), IntPoint(0, 0)));
    point_.push_back(std::make_pair(IntPoint(255, 0), IntPoint(255, 0)));
    point_.push_back(std::make_pair(IntPoint(0, 255), IntPoint(0, 255)));
    point_.push_back(std::make_pair(IntPoint(255, 255), IntPoint(255, 255)));
    point_.push_back(std::make_pair(IntPoint(128, 0), IntPoint(128, 20)));
    point_.push_back(std::make_pair(IntPoint(0, 128), IntPoint(20, 128)));
    point_.push_back(std::make_pair(IntPoint(128, 255), IntPoint(128, 235)));
    point_.push_back(std::make_pair(IntPoint(255, 128), IntPoint(235, 128)));
}

void ImageWidget::paintEvent(QPaintEvent *paintevent) {
  QPainter painter;
  painter.begin(this);

  // Draw background
  painter.setBrush(Qt::lightGray);
  QRect back_rect(0, 0, width(), height());
  painter.drawRect(back_rect);

  // Draw image
  QImage temp;
  // int w = ptr_image_->width(), h = ptr_image_->height();
  // if (w > width()) {
  //   h = width() * h / w;
  //   w = width();
  // }
  // if (h > height()) {
  //   w = height() * w / h;
  //   h = height();
  // }
  // temp = (*ptr_image_).scaled(w, h, Qt::KeepAspectRatio);
  temp = *ptr_image_;
  QRect rect =
      QRect((width() - temp.width()) / 2, (height() - temp.height()) / 2,
            temp.width(), temp.height());
  painter.drawImage(rect, temp);

  img_center.setX((width() - temp.width()) / 2);
  img_center.setY((height() - temp.height()) / 2);

  this->drawTip(&painter, img_center.getX(), img_center.getY());
  // Save coord of the top left edge for later use

  for (auto point_pair : point_) {
    //(x1,y1)->(x2,y2)
    painter.drawLine(point_pair.first.getX() + img_center.getX(),
                     point_pair.first.getY() + img_center.getY(),
                     point_pair.second.getX() + img_center.getX(),
                     point_pair.second.getY() + img_center.getY());
  }

  painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent *event) {
  if (Qt::LeftButton == event->button()) {
    qDebug("mouse pressed, at (%d,%d)", event->pos().rx(), event->pos().ry());
    mouse_status_ = true;
    start_point_ = event->pos();
  }
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event) {
  if (mouse_status_)
    end_point_ = event->pos();
  last_mouse = event->pos();
  update();
}

void ImageWidget::drawTip(QPainter *p, int aleft, int atop) {
  //char buf[100];
  //const QRect rectangle = QRect(0, 0, 300, 100);
  //QRect boundingRect;
  //sprintf_s(
  //    buf,
  //    sizeof(buf),
  //    "X: %d, Y: %d (mapped X: %d, Y: %d)\n"
  //    "Map: (%d,%d)->(%d,%d)",
  //    last_mouse.rx(), last_mouse.ry(), last_mouse.rx() - aleft,
  //    last_mouse.ry() - atop, last_mouse.rx() - aleft, last_mouse.ry() - atop,
  //    pnt_map[std::make_pair(last_mouse.rx() - aleft, last_mouse.ry() - atop)]
  //        .first,
  //    pnt_map[std::make_pair(last_mouse.rx() - aleft, last_mouse.ry() - atop)]
  //        .second);
  //p->drawText(rectangle, 0, tr(buf), &boundingRect);
}

void ImageWidget::clearDots() { point_.clear(); }

void ImageWidget::Open() {
  // Open file
  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

  // Load file
  if (!fileName.isEmpty()) {
    ptr_image_->load(fileName);
    *(ptr_image_backup_) = *(ptr_image_);
  }

  // ptr_image_->invertPixels(QImage::InvertRgb);
  //*(ptr_image_) = ptr_image_->mirrored(true, true);
  //*(ptr_image_) = ptr_image_->rgbSwapped();
  cout << "image size: " << ptr_image_->width() << ' ' << ptr_image_->height()
       << endl;
  update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *event) {
  qDebug("Adding (%d,%d)->(%d,%d) (orig: (%d,%d)->(%d,%d)), total %d now.",
         start_point_.rx() - img_center.getX(),
         start_point_.ry() - img_center.getY(),
         end_point_.rx() - img_center.getX(),
         end_point_.ry() - img_center.getY(), start_point_.rx(),
         start_point_.ry(), end_point_.rx(), end_point_.ry(),
         point_.size() + 1);

  IntPoint p;
  p.setX(start_point_.rx() - img_center.getX())
      .setY(start_point_.ry() - img_center.getY());
  IntPoint q;
  q.setX(end_point_.rx() - img_center.getX())
      .setY(end_point_.ry() - img_center.getY());
  point_.push_back(std::make_pair(p, q));
  mouse_status_ = false;
  update();
}

void ImageWidget::Save() { SaveAs(); }

void ImageWidget::SaveAs() {
  QString filename = QFileDialog::getSaveFileName(
      this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
  if (filename.isNull()) {
    return;
  }

  ptr_image_->save(filename);
}

void ImageWidget::Invert() {
   for (int i = 0; i < ptr_image_->width(); i++) {
     for (int j = 0; j < ptr_image_->height(); j++) {
       QRgb color = ptr_image_->pixel(i, j);
       ptr_image_->setPixel(
           i, j,
           qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
     }
   }

   // equivalent member function of class QImage
   // ptr_image_->invertPixels(QImage::InvertRgb);
   update();

}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical) {
  QImage image_tmp(*(ptr_image_));
  int width = ptr_image_->width();
  int height = ptr_image_->height();

  if (ishorizontal) {
    if (isvertical) {
      for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
          ptr_image_->setPixel(i, j,
                               image_tmp.pixel(width - 1 - i, height - 1 - j));
        }
      }
    } else {
      for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
          ptr_image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
        }
      }
    }
  } else {
    if (isvertical) {
      for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
          ptr_image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
        }
      }
    }
  }

  // equivalent member function of class QImage
  //*(ptr_image_) = ptr_image_->mirrored(true, true);
  update();
}

void ImageWidget::TurnGray() {
  for (int i = 0; i < ptr_image_->width(); i++) {
    for (int j = 0; j < ptr_image_->height(); j++) {
      QRgb color = ptr_image_->pixel(i, j);
      int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
      ptr_image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
    }
  }

  update();
}

void ImageWidget::Restore() {
  *(ptr_image_) = *(ptr_image_backup_);
  this->clearDots();
  update();
}

void ImageWidget::IDWWarp() {
    doWarp<IDWWarper>();
}

void ImageWidget::RBFWarp() {
    doWarp<RBFWarper>();
}