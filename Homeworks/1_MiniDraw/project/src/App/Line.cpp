#include "Line.h"

using namespace DrawContext;

Line::Line()
{
	this->setAttr("point-required", 2);
}

Line::~Line()
{
}

void Line::Draw(QPainter& painter)
{
	if (ctrl_points.size() == 2) {
		painter.drawLine(ctrl_points[0], ctrl_points[1]);
	}
}
