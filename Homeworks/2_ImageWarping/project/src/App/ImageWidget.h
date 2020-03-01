#pragma once
#include "ImageWarper.h"
#include <QWidget>
#include <vector>
#include <map>

QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

class ImageWidget : public QWidget {
  Q_OBJECT

public:
  ImageWidget(void);
  ~ImageWidget(void);

  

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

protected:
  void paintEvent(QPaintEvent *paintevent);

  template <typename Warper>
  void doWarp(void) {
      QImage image_new(ptr_image_->width(), ptr_image_->height(), ptr_image_->format());
      image_new.fill(QColor(0, 0, 0));

      Warper::getInstance()->initialize(point_);

      int w = ptr_image_->width();
      int h = ptr_image_->height();
      for (int i = 0; i < ptr_image_->width(); i++) {
          for (int j = 0; j < ptr_image_->height(); j++) {
              QColor clr = ptr_image_->pixelColor(i, j);
              IntPoint p, q;
              p.setX(i).setY(j);

              q = Warper::getInstance()->doTrans(p);
              if (q.getX() >= w || q.getY() >= h || q.getX() < 0 || q.getY() < 0) {
                  continue;
              }

              image_new.setPixelColor(q.getX(), q.getY(), clr);

          }
      }

      *(ptr_image_) = image_new;
      update();
  }

public slots:
  // File IO
  void Open();   // Open an image file, support ".bmp, .png, .jpg" format
  void Save();   // Save image to current file
  void SaveAs(); // Save image to another file

  // Image processing
  void Invert(); // Invert pixel value in image
  void Mirror(bool horizontal = false,
              bool vertical = true); // Mirror image vertically or horizontally
  void TurnGray();                   // Turn image to gray-scale map
  void Restore();                    // Restore image to origin
  void IDWWarp();
  void RBFWarp();
  void clearDots();
  void loadDebug();

private:
  QImage *ptr_image_; // image
  QImage *ptr_image_backup_;
  void drawTip(QPainter *p, int aleft, int atop);

  bool mouse_status_;
  QPoint start_point_;
  QPoint end_point_;

  std::vector<IntMapPoint> point_;
  QPoint last_mouse;
  IntPoint img_center;
  std::map<std::pair<int, int>, std::pair<int, int>> pnt_map;
};
