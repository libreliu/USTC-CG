#pragma once

#include "shape.h"

class Ellipse : public shape {
public:
	Ellipse();
	~Ellipse();

	void Draw(QPainter& painter);
};

