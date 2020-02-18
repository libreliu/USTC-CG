#pragma once

#include "Shape.h"

class CEllipse : public Shape {
public:
	CEllipse();
	~CEllipse();

	void Draw(QPainter& painter);
};

