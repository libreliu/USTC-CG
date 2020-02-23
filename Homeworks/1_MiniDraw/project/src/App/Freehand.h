#pragma once

#include "Shape.h"

namespace DrawContext {

	class Freehand :public Shape
	{
	public:
		Freehand();
		~Freehand();


		void Draw(QPainter& painter) override;
	private:
		std::vector<QPoint> path;
	};
}

