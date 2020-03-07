#pragma once

#include"Shape.h"
namespace DrawContext {
	class Rect :public Shape
	{
	public:
		Rect();
		~Rect();

		void Draw(QPainter& painter);
		virtual Eigen::Matrix<int, -1, -1> getMaskMatrix() override;
	};
}
