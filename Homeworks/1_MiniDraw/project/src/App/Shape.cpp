#include "Shape.h"

using namespace DrawContext;

Shape::Shape()
{
}

Shape::~Shape()
{
}

void Shape::addCtrlPoint(const QPoint& p) {
	ctrl_points.push_back(p);
}

const QPoint& Shape::getLastCtrlPoint() {
	if (ctrl_points.size() == 0) {
		throw std::out_of_range("nothing to get");
	}
	return ctrl_points[ctrl_points.size() - 1];
}

void Shape::modifyLastCtrlPoint(const QPoint& p) {
	if (ctrl_points.size() == 0) {
		throw std::out_of_range("nothing to modify");
		return;
	}
	ctrl_points[ctrl_points.size() - 1] = p;
}

void Shape::drawCross(QPainter& paint, int x, int y) {
	const char icon[5][5] = {
		{ 0, 0, 1, 0, 0 },
		{ 0, 0, 1, 0, 0 },
		{ 1, 1, 1, 1, 1 },
		{ 0, 0, 1, 0, 0 },
		{ 0, 0, 1, 0, 0 }
	};

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			int adjusted_x = x - 2 + i;
			int adjusted_y = y - 2 + j;
			
			// todo: add sanity check
			if (icon[i][j] != 0) {
				paint.drawPoint(adjusted_x, adjusted_y);
			}
		}
	}

}

void Shape::DrawCtrlPoints(QPainter& paint)
{
	for (auto &p: ctrl_points) {
		this->drawCross(paint, p.x(), p.y());
	}
}

void Shape::setAttr(const std::string &s, int a)
{
	attr[s] = a;
}

int Shape::getAttr(const std::string &s)
{
	auto it = attr.find(s);
	if (attr.end() != it) {
		return it->second;
	} else {
		return -1;
	}
}

#define my_max(a, b) ((a >= b ? a : b))
#define my_min(a, b) ((a >= b ? b : a))

int Shape::getCtrlPoint(const QPoint& cursor, int range)
{
	const int range_x = range;
	const int range_y = range;
	const int xmin = my_max(cursor.x() - range_x, 0);
	// todo sanity check
	const int xmax = cursor.x() + range_x;
	const int ymin = my_max(cursor.y() - range_y, 0);
	const int ymax = cursor.y() + range_y;

	for (int i = 0; i < ctrl_points.size(); i++) {
		QPoint &p = ctrl_points[i];
		if (p.x() >= xmin && p.x() <= xmax && p.y() >= ymin && p.y() <= ymax) {
			return i;
		}
	}
	return -1;
}

#undef my_min
#undef my_max

const std::vector<QPoint>& Shape::getCtrlPoints(void)
{
	return ctrl_points;
}

void Shape::modifyCtrlPoint(int index, const QPoint& dest)
{
	if (index >= ctrl_points.size()) {
		throw std::out_of_range("modifyed an out-of-range index");
	}
	ctrl_points[index] = dest;
}

