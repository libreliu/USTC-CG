#include "Rect.h"

using namespace DrawContext;

Rect::Rect()
{
	this->setAttr("point-required", 2);
	this->setAttr("mask-matrix-valid", 1);
	this->mask_mat_valid = false;
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

const Eigen::Matrix<int, -1, -1> &DrawContext::Rect::getMaskMatrix()
{
	assert(ctrl_points.size() == 2);
	if (mask_mat_valid) {
		return mask_mat;
	} else {
		QRect rect = getBoundingRect();
		mask_mat = Eigen::Matrix<int, -1, -1>::Ones(rect.height(), rect.width());
		mask_mat_valid = true;
		return mask_mat;
	}
}
