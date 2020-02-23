#include "Freehand.h"

using namespace DrawContext;

Freehand::Freehand() {
    this->setAttr("raw-mode", 1);
}

Freehand::~Freehand() {

}

void Freehand::Draw(QPainter& painter) {
    if (ctrl_points.size() >= 1) {
        auto it = ctrl_points.begin();
        for (; it != ctrl_points.end() && (it + 1) != ctrl_points.end(); it++) {
            painter.drawLine(*it, *(it+1));
        }
    }
}
