#include "Polygon.h"
#include <cstdio>

Polygon::Polygon() : finished(false) {

}

Polygon::~Polygon() {

}

void Polygon::set_start(QPoint s)
{
	path.push_back(s);
}

void Polygon::set_end(QPoint e)
{
    printf("set_end called\n");
    end = e;
}


void Polygon::set_finished(void) {
    this->finished = true;
}

// 极端情况：只有一个点，那就是点-点了
void Polygon::Draw(QPainter& painter) {
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