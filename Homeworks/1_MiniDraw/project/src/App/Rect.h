#pragma once

#include"Shape.h"
namespace DrawContext {
	class Rect :public Shape
	{
	public:
		Rect();
		~Rect();

		void Draw(QPainter& painter);
	};
}
