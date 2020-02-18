#pragma once

#include "shape.h"

class Polygon :public shape
{
public:
	Polygon();
	~Polygon();

    void set_finished(void);

	virtual void set_start(QPoint s) override;
	virtual void set_end(QPoint e) override;

	void Draw(QPainter& painter);
private:
    // not including end
	std::vector<QPoint> path;
    bool finished;
};
