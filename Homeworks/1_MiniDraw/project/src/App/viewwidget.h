#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>
#include <qevent.h>
#include <qpainter.h>
#include "ui_viewwidget.h"
#include "Shape.h"
#include "Line.h"
#include "Rect.h"
#include "Freehand.h"
#include "Ellipse.h"
#include "Polygon.h"
#include <vector>
#include "ShapeManager.h"

class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget *parent = 0);
	~ViewWidget();

	typedef enum mode {
		VIEW = 0,
		DRAG_WHOLE = 1,  // Whole Drag Mode
		DRAG_POINT = 2,  // Point Drag Mode
		ADD_POINT = 3,   // Point Add mode
		RAW = 4          // Raw mode (the original interactions)
	} mode;

private:
	Ui::ViewWidget ui;

	// -- previous elements --
	std::vector<DrawContext::Shape *> shape_list;
	
	// current working shape
	DrawContext::Shape *shape_working;
	int shape_working_points;

	// mode
	int cur_mode;
	int cur_shape;

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
