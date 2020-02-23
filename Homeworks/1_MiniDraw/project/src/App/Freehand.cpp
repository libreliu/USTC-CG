#include "Freehand.h"

using namespace DrawContext;

Freehand::Freehand() {

}

Freehand::~Freehand() {

}

void Freehand::Draw(QPainter& painter) {
    if (path.size() >= 1) {
        auto it = path.begin();
        for (; it != path.end() && (it + 1) != path.end(); it++) {
            painter.drawLine(*it, *(it+1));
        }
    }
}
