#include "Polygon.h"

CPolygon::CPolygon() : finished(false) {

}

CPolygon::~CPolygon() {

}

void CPolygon::set_start(QPoint s)
{
	path.push_back(s);
}

void CPolygon::set_end(QPoint e)
{
    end = e;
}


void CPolygon::set_finished(void) {
    this->finished = true;
}

// 极端情况：只有一个点，那就是点-点了
void CPolygon::Draw(QPainter& painter) {
    // path -> end || end
    if (path.size() >= 1) {
        auto it = path.begin();
        for (; it != path.end() && (it + 1) != path.end(); it++) {
            painter.drawLine(*it, *(it+1));
        }

        if (finished) {
            painter.drawLine(path.back(), path.front());
        } else {
            painter.drawLine(path.back(), end);
        }
    }
}