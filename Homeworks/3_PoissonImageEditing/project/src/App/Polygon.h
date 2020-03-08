#pragma once

#include "Shape.h"

namespace DrawContext {

	class Polygon :public Shape
	{
	public:
		Polygon();
		~Polygon();

		void Draw(QPainter& painter);
		const Eigen::Matrix<int, -1, -1> &getMaskMatrix() override;
	};

}
