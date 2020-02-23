#pragma once

#include "Shape.h"

namespace DrawContext {

	class Polygon :public Shape
	{
	public:
		Polygon();
		~Polygon();

		void set_finished(void);

		void Draw(QPainter& painter);
	private:
		// not including end
		std::vector<QPoint> path;
		bool finished;
	};

}
