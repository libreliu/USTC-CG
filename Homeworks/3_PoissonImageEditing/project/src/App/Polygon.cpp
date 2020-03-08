#include "Polygon.h"
#include <assert.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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
    
	if (mask_mat_valid && bounding_box_valid && false) {
		return mask_mat;
	} else {
		QRect rect = getBoundingRect();

        if (rect.width() < 2 && rect.height() < 2) {
            printf("Warning: rect too small\n");
            mask_mat_valid = false;
            throw mask_too_small();
        } else {
            assert(this->ctrl_points.size() >= 2);
            cv::Mat mask_mat_cv = cv::Mat::zeros(rect.width(), rect.height(), CV_8UC1);
            mask_mat = Eigen::Matrix<int, -1, -1>::Zero(rect.height(), rect.width());

            int x_topleft = rect.topLeft().x() - 1;
            int y_topleft = rect.topLeft().y() - 1 ;

            cv::Point* cv_points = new cv::Point[this->ctrl_points.size()];
            int i_c = 0;
            for (auto& p : ctrl_points) {
                cv_points[i_c] = cv::Point(p.x() - x_topleft, p.y() - y_topleft);
                i_c++;
            }
            const cv::Point* ppt[1] = { cv_points };
            int npt[1] = { i_c };

            cv::fillPoly(mask_mat_cv, ppt, npt, 1, cv::Scalar(255));

            for (int i = 0; i < rect.height(); i++) {
                for (int j = 0; j < rect.width(); j++) {
                    if (mask_mat_cv.at<uchar>(i, j) > 0) {
                        mask_mat(i, j) = 1;
                    }
                }
            }

            delete[] cv_points;
            mask_mat_valid = true;
        }
		return mask_mat;
	}
}