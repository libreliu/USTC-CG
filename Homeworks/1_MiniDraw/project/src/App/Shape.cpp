#include "Shape.h"

using namespace DrawContext;

Shape::Shape()
{
}

Shape::~Shape()
{
}

void Shape::DrawCtrlPoints(QPainter& paint)
{
}

void Shape::setAttr(std::string &s, int a)
{
	attr[s] = a;
}

int Shape::getAttr(std::string &s)
{
	auto it = attr.find(s);
	if (attr.end() != it) {
		return it->second;
	} else {
		return -1;
	}
}

int Shape::getCtrlPoint(QPoint& cursor, int range)
{
	return 0;
}

const std::vector<QPoint>& Shape::getCtrlPoints(void)
{
	// TODO: 在此处插入 return 语句
	return ctrl_points;
}

void Shape::modifyCtrlPoint(int index, QPoint& dest)
{
}

