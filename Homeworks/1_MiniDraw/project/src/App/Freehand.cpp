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
    if (path.size() >= 1) {
        auto it = path.begin();
        for (; it != path.end() && (it + 1) != path.end(); it++) {
            painter.drawLine(*it, *(it+1));
        }
    }
}
