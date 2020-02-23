#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include "Ellipse.h"
#include "Freehand.h"
#include "Line.h"
#include "Polygon.h"
#include "Rect.h"
#include "Shape.h"
#include "ShapeManager.h"
#include "ui_viewwidget.h"
#include <QWidget>
#include <qevent.h>
#include <qpainter.h>
#include <vector>

// Put this to cease the dependency chain
class MiniDraw;

class ViewWidget : public QWidget {
  Q_OBJECT

public:
  ViewWidget(QWidget *parent);
  ~ViewWidget();

  typedef enum mode {
    VIEW = 0,
    DRAG_WHOLE = 1, // Whole Drag Mode
    DRAG_POINT = 2, // Point Drag Mode
    ADD_POINT = 3,  // Point Add mode
    RAW = 4         // Raw mode (the original interactions)
  } mode;

private:
  Ui::ViewWidget ui;

  // -- previous elements --
  std::vector<DrawContext::Shape *> shape_list;

  // current working shape
  DrawContext::Shape *shape_working;
  int shape_working_points;

  // mode
  mode cur_mode;
  int cur_shape;
  int next_shape; // the shape that is supposed to draw next
  mode next_mode; // goal: entering every mode makes shape_working_points=0 (and
                  // no shape drawing)

  // parent-in-law
  MiniDraw *parent_in_law;

  void sendWarning(const QString s);
  void doModeSwitch();

  void pushShape();
  void clearShape();

public:
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void paintEvent(QPaintEvent *);

  int getCurShape();

  // use these when refreshing status, and status bar will also get called

public slots:
  void setShape(int s);
  void setMode(mode m);
};

#endif // VIEWWIDGET_H
