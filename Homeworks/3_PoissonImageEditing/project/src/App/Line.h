#pragma once

#include "Shape.h"

namespace DrawContext {

	class Line :public Shape
	{
	public:
		Line();
		~Line();

		void Draw(QPainter& painter);
	};
}

