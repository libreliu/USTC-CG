#pragma once

#include "Shape.h"

namespace DrawContext {
	class Ellipse : public Shape {
	public:
		Ellipse();
		~Ellipse();

		void Draw(QPainter& painter);
	};

};