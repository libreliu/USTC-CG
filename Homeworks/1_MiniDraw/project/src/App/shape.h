#pragma once

#include <QtGui>

class Shape
{
public:
	shape();
	virtual ~shape();
	virtual void Draw(QPainter &paint)=0;
	virtual void set_start(QPoint s);
	virtual void set_end(QPoint e);

public:
	enum Type
	{
		kDefault = 0,
		kLine = 1,
		kRect = 2,
		kFreehand = 3,
	};
	
public:
	QPoint start;
	QPoint end;
};

