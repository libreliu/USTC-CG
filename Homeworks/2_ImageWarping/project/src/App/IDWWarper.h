#pragma once

#include "ImageWarper.h"
#include <vector>

// class IDWWarper : ImageWarper {
// public:

//     using TMatrix = struct {
//         double t11;
//         double t12;
//         double t21;
//         double t22;
//     };

//     using IntPoint = IPoint<int>;
//     using IntMapPoint = std::pair<IntPoint, IntPoint>;

//     virtual const IntPoint& doTrans(const IntPoint &p) override;
//     virtual void initialize(std::vector<IntMapPoint> v) override;

//     static IDWWarper *getInstance();

// protected:
//     std::vector<TMatrix> t_mat;
//     std::vector<IntMapPoint> ctrl_point_pair;
//     int total_pairs;

// };

class IDWWarper {
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

    static IDWWarper *getInstance();

protected:
    std::vector<TMatrix> t_mat;
    std::vector<IntMapPoint> ctrl_point_pair;
    int total_pairs;

    static IDWWarper inst;
};