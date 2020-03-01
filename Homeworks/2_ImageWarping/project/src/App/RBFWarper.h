#pragma once
#include "ImageWarper.h"
#include <Eigen/Dense>
#include <vector>

class RBFWarper {
public:

    using TMatrix = struct {
        double t11;
        double t12;
        double t21;
        double t22;
    };

    using IntPoint = IPoint<int>;
    using IntMapPoint = std::pair<IntPoint, IntPoint>;

    const IntPoint& doTrans(const IntPoint &p);
    void initialize(std::vector<IntMapPoint> v);

    static RBFWarper *getInstance();

protected:
    std::vector<Eigen::Vector2d> a_vecs;
    std::vector<IntMapPoint> ctrl_point_pair;
    size_t total_pairs;

    Eigen::Matrix2d M_mat;
    Eigen::Vector2d b_vec;

    static RBFWarper inst;
};