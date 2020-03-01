#pragma once

#include <map>
#include <unordered_map>

// image warper: give x and get y
template <typename T>
class IPoint {

public:
    const T& getX() const {
        return _x;
    };
    const T& getY() const {
        return _y;
    };
    int operator<(const IPoint &other) const {
        return (this->_x < other._x);
    }
    IPoint<T> &setX(const T& x) {
        _x = x;
        return *this;
    }
    IPoint<T> &setY(const T& y) {
        _y = y;
        return *this;
    }
    IPoint() {}
    IPoint(const T x, const T y) : _x(x), _y(y) {}

private:
    T _x;
    T _y;
};


using IntPoint = IPoint<int>;
using IntMapPoint = std::pair<IntPoint, IntPoint>;

// class ImageWarper {
// public:

//     using IntPoint = IPoint<int>;
//     using IntMapPoint = std::pair<IntPoint, IntPoint>;

//     virtual const IPoint<int>& doTrans(const IntPoint &p) = 0;
//     virtual void initialize(std::map<IntPoint, IntPoint> v) = 0;

// };