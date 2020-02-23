#include "Ellipse.h"
#include <QRect>

using namespace DrawContext;

Ellipse::Ellipse()
{
	this->setAttr("point-required", 2);
	printf("Ellipse constructed\n");
}


Ellipse::~Ellipse()
{
}

void Ellipse::Draw(QPainter& painter)
{
	if (ctrl_points.size() == 2) {
		painter.drawEllipse(QRect(ctrl_points[0], ctrl_points[1]));
	}
}
