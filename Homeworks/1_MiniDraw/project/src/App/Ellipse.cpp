#include "Ellipse.h"
#include <QRect>

CEllipse::CEllipse()
{
}


CEllipse::~CEllipse()
{
}

void CEllipse::Draw(QPainter& painter)
{
	painter.drawEllipse(QRect(start, end));
}
