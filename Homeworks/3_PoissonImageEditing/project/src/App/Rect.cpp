#include "Rect.h"

using namespace DrawContext;

Rect::Rect()
{
	this->setAttr("point-required", 2);
	this->setAttr("mask-matrix-valid", 1);
}

Rect::~Rect()
{
}

void Rect::Draw(QPainter& painter)
{
	if (ctrl_points.size() == 2) {
		painter.drawRect(QRect(ctrl_points[0], ctrl_points[1]));
	}
}

Eigen::Matrix<int, -1, -1> DrawContext::Rect::getMaskMatrix()
{
	
	return Eigen::Matrix<int, -1, -1>();
}
