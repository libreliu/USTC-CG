#include "Poisson.h"


void Poisson::setOrig(const QImage &orig, const Eigen::Matrix<int, -1, -1>& mask) {
    int w = mask.cols();
    int h = mask.rows();

    assert(orig.width() == w + 2 && orig.height() == h + 2);
    this->mask = mask;
    this->orig = orig;
    
    int m = w * h;
    int n = m;
    coeff_mat = Eigen::SparseMatrix<double>(m, n);
    coeff_mat.setZero();

// Wrap out-of-bound points to 
#define GET_INDEX(p, q) ((p * w + q))
#define IS_INNER_OR_BOUNDARY_AND_WITHIN_RANGE(p, q) (( p >= 0 && p < h && q >= 0 && q < w && mask(p, q) > 0))

    // -------j
    // | MASK 
    // | AREA
    // i
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (mask(i, j) != 1) {       // Boundary (4-way, not 8-way) or exterior point
                continue;
            }
            int current_row = GET_INDEX(i, j);
            coeff_mat.insert(current_row, GET_INDEX(i, j)) = 4;
            // Up
            if (IS_INNER_OR_BOUNDARY_AND_WITHIN_RANGE(i - 1, j)) {
                coeff_mat.insert(current_row, GET_INDEX(i - 1, j)) = -1;
            } else {
                // it's a boundary point (and (i, j) point is boundary actually)
                // left for vec. assignment
            }

            // Down
            if (IS_INNER_OR_BOUNDARY_AND_WITHIN_RANGE(i + 1, j)) {
                coeff_mat.insert(current_row, GET_INDEX(i + 1, j)) = -1;
            }

            // Left
            if (IS_INNER_OR_BOUNDARY_AND_WITHIN_RANGE(i, j - 1)) {
                coeff_mat.insert(current_row, GET_INDEX(i, j - 1)) = -1;
            }

            // Right
            if (IS_INNER_OR_BOUNDARY_AND_WITHIN_RANGE(i, j + 1)) {
                coeff_mat.insert(current_row, GET_INDEX(i, j + 1)) = -1;
            }
        }
    }

    // compute, left solving for later
    solver.compute(coeff_mat);

    // calculate picture laplace
    // -------j
    // | MASK 
    // | AREA
    // i
    // rgb (not rgbF) used here
    laplace_r_orig = laplace_g_orig = laplace_b_orig = Eigen::MatrixXd(mask.rows(), mask.cols());
    // Image(1,1) <-> Mat(0,0)
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            QColor center = orig.pixelColor(i + 1, j + 1);
            QColor up = orig.pixelColor(i + 1 - 1, j + 1);
            QColor down = orig.pixelColor(i + 1 + 1, j + 1);
            QColor left = orig.pixelColor(i + 1, j + 1 - 1);
            QColor right = orig.pixelColor(i + 1, j + 1 + 1);

            laplace_r_orig(i, j) = up.red() + down.red() + left.red() + right.red() - 4 * center.red();
            laplace_g_orig(i, j) = up.green() + down.green() + left.green() + right.green() - 4 * center.green();
            laplace_b_orig(i, j) = up.blue() + down.blue() + left.blue() + right.blue() - 4 * center.blue();
        }
    }
    
    // sum up terms in b_vec except for boundary ones now
    // -------j
    // | MASK 
    // | AREA
    // i

    b_vec_r = b_vec_g = b_vec_b = Eigen::VectorXd(w * h);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int current_row = GET_INDEX(i, j);
            b_vec_r(current_row) = b_vec_g(current_row) = b_vec_b(current_row) = 0;

            // Notice: in paper's finital differentiation scheme, all coeffs are reverted
            // Here i'm conforming to the paper
            b_vec_r(current_row) += -laplace_r_orig(i, j);
            b_vec_g(current_row) += -laplace_g_orig(i, j);
            b_vec_b(current_row) += -laplace_b_orig(i, j);

        }
    }

}

// Qt coord system
// ----x
// |
// |
// y

// TODO: FIX ALL THE COORD! I MAY HAVE CONFUSED X AND Y..

const QImage &Poisson::doTransform(const QImage &dest) {

    int w = mask.cols();
    int h = mask.rows();
    assert(dest.width() == w + 2 && dest.height() == h + 2);

    // make up the remaining pieces for b_vec
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int current_row = GET_INDEX(i, j);

#define IS_EXTERIOR(p, q) (( (p < 0 || p >= h) || (q < 0 || q >= w) || mask(p, q) == 0 ))

            // Up
            if (IS_EXTERIOR(i - 1, j)) {
                QColor up = dest.pixelColor(i + 1 - 1, j + 1);
                b_vec_r(current_row) += up.red();
                b_vec_g(current_row) += up.green();
                b_vec_b(current_row) += up.blue();
            }

            // Down
            if (IS_EXTERIOR(i + 1, j)) {
                QColor down = dest.pixelColor(i + 1 + 1, j + 1);
                b_vec_r(current_row) += down.red();
                b_vec_g(current_row) += down.green();
                b_vec_b(current_row) += down.blue();
            }

            // Left
            if (IS_EXTERIOR(i, j - 1)) {
                QColor left = dest.pixelColor(i + 1, j + 1 - 1);
                b_vec_r(current_row) += left.red();
                b_vec_g(current_row) += left.green();
                b_vec_b(current_row) += left.blue();
            }

            // Right
            if (IS_EXTERIOR(i, j + 1)) {
                QColor right = dest.pixelColor(i + 1, j + 1 + 1);
                b_vec_r(current_row) += right.red();
                b_vec_g(current_row) += right.green();
                b_vec_b(current_row) += right.blue();
            }
        }
    }

    // Solve the equation
    Eigen::VectorXd result_r_vec = solver.solve(b_vec_r);
    Eigen::VectorXd result_g_vec = solver.solve(b_vec_g);
    Eigen::VectorXd result_b_vec = solver.solve(b_vec_b);

    // Assign the image, at this point it's allocated but uninitialized
    result = QImage(orig.width(), orig.height(), orig.format());

    // Copy the top border
    for (int m = 0; m < orig.width(); m++) {
        result.setPixelColor(m, 0, )
    }

    for (int i = 1; i < orig.height(); i++) {
        for (int j = 0; j < orig.width(); j++) {

        }
    }
}