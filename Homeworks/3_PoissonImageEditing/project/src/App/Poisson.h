#pragma once

#include <QImage>
#include <Eigen/Eigen>
#include <Eigen/SparseQR>
#include <assert.h>

// 1. Orig img + mask + Dest img = new img
//    mask are w*h, but orig img are (w+2)*(h+2)
// 2. dest img can be reloaded, and new img can be calculated instantly
class Poisson {
public:
    void setOrig(const QImage &orig, const Eigen::Matrix<int, -1, -1>& mask);
    const QImage &doTransform(const QImage &dest);

private:
    // w+2 * h+2, the border is required to be larger
    // and format is conforming to orig.format
    QImage result;
    // w+2 * h+2
    QImage orig;
    // w * h
    Eigen::Matrix<int, -1, -1> mask;
    Eigen::SparseMatrix<double> coeff_mat;
    Eigen::VectorXd b_vec_r;
    Eigen::VectorXd b_vec_g;
    Eigen::VectorXd b_vec_b;
    Eigen::MatrixXd laplace_r_orig;
    Eigen::MatrixXd laplace_g_orig;
    Eigen::MatrixXd laplace_b_orig;
    Eigen::SparseQR<Eigen::SparseMatrix<double>, Eigen::COLAMDOrdering<int>> solver;

    // 0 ; 1 .... width_total - 2; width_total - 1 
    int width_total;
    int height_total;
};