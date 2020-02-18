#pragma once

#include "Shape.h"

class Ellipse : public Shape {
public:
	Ellipse();
	~Ellipse();

	void Draw(QPainter& painter);
};

