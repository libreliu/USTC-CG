#include "Polygon.h"

using namespace DrawContext;

Polygon::Polygon() : finished(false) {

}

Polygon::~Polygon() {

}

void Polygon::set_finished(void) {
    this->finished = true;
}

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
           // painter.drawLine(path.back(), end);
        }
    }
}