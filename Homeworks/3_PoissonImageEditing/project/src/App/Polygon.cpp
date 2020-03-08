#include "Polygon.h"
#include <assert.h>

#include <opencv2/core/core.hpp>

using namespace DrawContext;

Polygon::Polygon() {
    this->setAttr("mask-matrix-valid", 1);
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

    // Also Draw Bounding Rect, and fill it 
    QRect rect = getBoundingRect();
    painter.drawRect(rect);
}

// fillPoly(Mat& img, const Point** pts, const int* npts, int ncontours, const Scalar& color, int lineType=8, int shift=0, Point offset=Point() )

const Eigen::Matrix<int, -1, -1> &Polygon::getMaskMatrix() {
    
	if (mask_mat_valid) {
		return mask_mat;
	} else {
		QRect rect = getBoundingRect();

        if (rect.width() > 2 && rect.height() >= 2) {
            printf("Warning: rect too small\n");
            mask_mat_valid = false;
            throw mask_too_small();
        } else {
            cv::Mat mask_mat_cv;
            


            mask_mat = Eigen::Matrix<int, -1, -1>::Ones(rect.width(), rect.height());
            mask_mat_valid = true;
        }
		return mask_mat;
	}
}