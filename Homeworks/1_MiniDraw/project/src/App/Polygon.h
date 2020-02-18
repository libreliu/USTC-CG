#pragma once

#include "Shape.h"

class CPolygon :public Shape
{
public:
	CPolygon();
	~CPolygon();

    void set_finished(void);

	virtual void set_start(QPoint s) override;
	virtual void set_end(QPoint e) override;

	void Draw(QPainter& painter);
private:
    // not including end
	std::vector<QPoint> path;
    bool finished;
};
