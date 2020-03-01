#include "RBFWarper.h"
#include <math.h>
#include <vector>
#include <iostream>

#define DIST_2(x1, y1, x2, y2) (((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)))

#define G(x, y, pix, piy) (( sqrt( (x-pix)*(x-pix) + (y-piy)*(y-piy)) ))

// the static instance
RBFWarper RBFWarper::inst;

RBFWarper *RBFWarper::getInstance() {
    return &(inst);
}

const RBFWarper::IntPoint& RBFWarper::doTrans(const RBFWarper::IntPoint &p) {
    double px = p.getX();
    double py = p.getY();

    Eigen::Vector2d fp = Eigen::Vector2d::Zero(); 
    for (int i = 0; i < total_pairs; i++) {
        double pix = ctrl_point_pair[i].first.getX();
        double piy = ctrl_point_pair[i].first.getY();
        fp += a_vecs[i] * G(px, py, pix, piy);
    }

    // affinity part
    Eigen::Vector2d p_vec;
    p_vec(0) = px;
    p_vec(1) = py;
    fp += M_mat * p_vec + b_vec;
    
    static IntPoint ret;
    ret.setX((int)fp(0)).setY((int)fp(1));

    return ret;
}

// Note: need order here!
void RBFWarper::initialize(std::vector<IntMapPoint> m) {

    total_pairs = m.size();

    // affine calculations, tbc
    M_mat = Eigen::Matrix2d::Identity();
    // M_mat = Eigen::Matrix2d::Zero();
    b_vec = Eigen::Vector2d::Zero();

    if (total_pairs < 4) { // or LSE won't work
        throw std::out_of_range("Expect pairs >= 4");
    }

    // [m11 m12 m21 m22 bx by]
    Eigen::MatrixXd affine_lse(6, 6);
    Eigen::VectorXd affine_lse_rhs(6);
    affine_lse = Eigen::MatrixXd::Zero(6, 6);
    affine_lse_rhs = Eigen::VectorXd::Zero(6);

    for (int i = 0; i < total_pairs; i++) {
        double pix = m[i].first.getX();
        double piy = m[i].first.getY();
        double qix = m[i].second.getX();
        double qiy = m[i].second.getY();

        // rbf_m11
        affine_lse(0, 0) += 2 * pix * pix;
        affine_lse(0, 1) += 2 * pix * piy;
        affine_lse(0, 4) += 2 * pix;
        affine_lse_rhs(0) += 2 * pix * qix;

        // rbf_m12
        affine_lse(1, 0) += 2 * pix * piy;
        affine_lse(1, 1) += 2 * piy * piy;
        affine_lse(1, 4) += 2 * piy;
        affine_lse_rhs(1) += 2 * qix * piy;

        // rbf_m21
        affine_lse(2, 2) += 2 * pix * pix;
        affine_lse(2, 3) += 2 * pix * piy;
        affine_lse(2, 5) += 2 * pix;
        affine_lse_rhs(2) += 2 * pix * qiy;

        // rbf_m22
        affine_lse(3, 2) += 2 * pix * piy;
        affine_lse(3, 3) += 2 * piy * piy;
        affine_lse(3, 5) += 2 * piy;
        affine_lse_rhs(3) += 2 * piy * qiy;

        // rbf_bx
        affine_lse(4, 0) += 2 * pix;
        affine_lse(4, 1) += 2 * piy;
        affine_lse(4, 4) += 2;
        affine_lse_rhs(4) += 2 * qix;

        // rbf_by
        affine_lse(5, 2) += 2 * pix;
        affine_lse(5, 3) += 2 * piy;
        affine_lse(5, 5) += 2;
        affine_lse_rhs(5) += 2 * qiy;
    }
    
    Eigen::VectorXd sol_affine = affine_lse.colPivHouseholderQr().solve(affine_lse_rhs);
    M_mat(0, 0) = sol_affine(0);
    M_mat(0, 1) = sol_affine(1);
    M_mat(1, 0) = sol_affine(2);
    M_mat(1, 1) = sol_affine(3);
    b_vec(0) = sol_affine(4);
    b_vec(1) = sol_affine(5);

    // a_vec calculations
    Eigen::MatrixXd A(2 * total_pairs, 2 * total_pairs);
    A = Eigen::MatrixXd::Zero(2 * total_pairs, 2 * total_pairs);
    for (int i = 0; i < 2 * total_pairs; i = i + 2) {
        for (int j = 0; j < 2 * total_pairs; j = j + 2) {
            double pix = m[i / 2].first.getX();
            double piy = m[i / 2].first.getY();
            double pjx = m[j / 2].first.getX();
            double pjy = m[j / 2].first.getY();

            A(i, j) = G(pjx, pjy, pix, piy); // this is in fact symmetry..
            A(i+1, j+1) = A(i, j);
        }
    }
    
    Eigen::VectorXd b(2 * total_pairs);
    for (int i = 0; i < 2 * total_pairs; i = i + 2) {
        // calculate Mp + b_vec
        Eigen::Vector2d p_vec;
        p_vec(0) = m[i / 2].first.getX();
        p_vec(1) = m[i / 2].first.getY();

        Eigen::Vector2d offset = M_mat * p_vec + b_vec;

        b(i) = m[i / 2].second.getX() - offset(0);
        b(i+1) = m[i / 2].second.getY() - offset(1);
    }

    std::cout << A << std::endl;
    std::cout << b << std::endl;

    // solve for a_vec
    Eigen::VectorXd solution = A.colPivHouseholderQr().solve(b);
    a_vecs = std::vector<Eigen::Vector2d>(total_pairs);

    for (int i = 0; i < 2 * total_pairs; i = i + 2) {
        a_vecs[i / 2](0) = solution(i);
        a_vecs[i / 2](1) = solution(i+1);
    }

    // aux info
    ctrl_point_pair = m;

    // check if f(pi) = qi
    for (int i = 0; i < total_pairs; i++) {
        printf("i = %d, (%d, %d) ->", i ,ctrl_point_pair[i].first.getX(), ctrl_point_pair[i].first.getY());
        IntPoint pnt = doTrans(ctrl_point_pair[i].first);
        printf("(%d, %d) [expect (%d, %d)]\n", 
            pnt.getX(), pnt.getY(), ctrl_point_pair[i].second.getX(), ctrl_point_pair[i].second.getY());
    }
}