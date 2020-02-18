#include "Ellipse.h"
#include <QRect>

Ellipse::Ellipse()
{
}


Ellipse::~Ellipse()
{
}

void Ellipse::Draw(QPainter& painter)
{
	painter.drawEllipse(QRect(start, end));
}
