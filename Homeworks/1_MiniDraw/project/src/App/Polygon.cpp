#include "Polygon.h"

using namespace DrawContext;

Polygon::Polygon() {

}

Polygon::~Polygon() {

}

void Polygon::Draw(QPainter& painter) {
    if (ctrl_points.size() >= 1) {
        auto it = ctrl_points.begin();
        for (; it != ctrl_points.end() && (it + 1) != ctrl_points.end(); it++) {
            painter.drawLine(*it, *(it+1));
        }

        painter.drawLine(ctrl_points.back(), ctrl_points.front());

    }
}