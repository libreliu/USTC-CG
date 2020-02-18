#pragma once

#include"shape.h"

class Freehand :public shape
{
public:
	Freehand();
	~Freehand();

	virtual void set_start(QPoint s) override;
	virtual void set_end(QPoint e) override;

	void Draw(QPainter& painter) override;
private:
	std::vector<QPoint> path;
};

