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
#include<vector>

class ViewWidget : public QWidget
{
	Q_OBJECT

public:
	ViewWidget(QWidget *parent = 0);
	~ViewWidget();

	typedef enum mode {
		VIEW,
		DRAG_WHOLE,  // Whole Drag Mode
		DRAG_POINT,  // Point Drag Mode
		ADD_POINT£¬  // Point Add mode
	} mode;

private:
	Ui::ViewWidget ui;

	// set 
	int cur_type;

public:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

public:
	void paintEvent(QPaintEvent *);

public slots:
	void setType(int type);
	void setMode(mode m);
};

#endif // VIEWWIDGET_H
