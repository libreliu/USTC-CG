#include "Freehand.h"

Freehand::Freehand() {

}

Freehand::~Freehand() {

}

void Freehand::set_start(QPoint s)
{
	if (path.size() == 0) {
		path.push_back(s);
	}
}

void Freehand::set_end(QPoint e)
{
	if (path.back() != e) {
		path.push_back(e);
	}
}

void Freehand::Draw(QPainter& painter) {
	for (auto& p : this->path) {
		painter.drawPoint(p);
	}
}
