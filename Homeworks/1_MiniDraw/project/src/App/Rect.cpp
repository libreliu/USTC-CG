#include "Rect.h"

using namespace DrawContext;

Rect::Rect()
{
	this->setAttr("point-required", 2);
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
