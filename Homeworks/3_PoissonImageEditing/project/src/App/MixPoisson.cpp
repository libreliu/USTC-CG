#include "MixPoisson.h"
#include <iostream>

void MixPoisson::setOrig(const QImage &orig, const Eigen::Matrix<int, -1, -1>& mask) {
    int w = mask.cols();
    int h = mask.rows();

    printf("Copy start\n");
    assert(orig.width() == w + 2 && orig.height() == h + 2);
    this->mask = mask;
    this->orig = orig;
    printf("Copy complete\n");
    
    int m = w * h;
    int n = m;
    coeff_mat = Eigen::SparseMatrix<double>(m, n);
    coeff_mat.setZero();
    printf("Coeff matrix zeroed\n");
// Wrap out-of-bound points to 
#define GET_INDEX(p, q) ((((p)) * ((w)) + ((q))))
#define IS_INNER_OR_BOUNDARY_AND_WITHIN_RANGE(p, q) (( ((p)) >= 0 && ((p)) < ((h)) && ((q)) >= 0 && ((q)) < ((w)) && mask(((p)), ((q))) > 0))

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

    printf("Coeff matrix build complete\n");
    // compute, left solving for later
    coeff_mat.makeCompressed();
    solver.compute(coeff_mat);

    printf("Coeff matrix solve complete\n");

    // sum up terms in b_vec except for boundary ones now
    // -------j
    // | MASK 
    // | AREA
    // i

    b_vec_r = b_vec_g = b_vec_b = Eigen::VectorXd::Zero(w * h);

    printf("b vector zeroed\n");

}

// Qt coord system
// ----x
// |
// |
// y

// TODO: FIX ALL THE COORD! I MAY HAVE CONFUSED X AND Y..

const QImage &MixPoisson::doTransform(const QImage &dest) {

    int w = mask.cols();
    int h = mask.rows();
    assert(dest.width() == w + 2 && dest.height() == h + 2);

    printf("mask: %d * %d\n", mask.rows(), mask.cols());
    printf("orig: width=%d, height=%d\n", orig.width(), orig.height());
    printf("dest: width=%d, height=%d\n", dest.width(), dest.height());

    // make up the remaining pieces for b_vec
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int current_row = GET_INDEX(i, j);

#define IS_EXTERIOR(p, q) (( (((p)) < 0 || ((p)) >= ((h))) || (((q)) < 0 || ((q)) >= ((w))) || mask(((p)), ((q))) == 0 ))

            if (mask(i, j) != 1) {       // Boundary (4-way, not 8-way) or exterior point
                continue;
            }

            // Gradients --- Ugly pieces ----
            QColor up_orig = orig.pixelColor(j + 1, i + 1 - 1);
            QColor up_dest = dest.pixelColor(j + 1, i + 1 - 1);            
            
            QColor center_orig = orig.pixelColor(j + 1, i + 1);
            QColor center_dest = dest.pixelColor(j + 1, i + 1);

            QColor down_dest = dest.pixelColor(j + 1, i + 1 + 1);
            QColor down_orig = orig.pixelColor(j + 1, i + 1 + 1);

            QColor left_dest = dest.pixelColor(j + 1 - 1, i + 1);
            QColor left_orig = orig.pixelColor(j + 1 - 1, i + 1);

            QColor right_dest = dest.pixelColor(j + 1 + 1, i + 1);
            QColor right_orig = orig.pixelColor(j + 1 + 1, i + 1);
    
            double upgrad_orig_red = center_orig.red() - up_orig.red();
            double upgrad_dest_red = center_dest.red() - up_dest.red();

            double downgrad_orig_red = center_orig.red() - down_orig.red();
            double downgrad_dest_red = center_dest.red() - down_dest.red();
            
            double leftgrad_orig_red = center_orig.red() - left_orig.red();
            double leftgrad_dest_red = center_dest.red() - left_dest.red();
                        
            double rightgrad_orig_red = center_orig.red() - right_orig.red();
            double rightgrad_dest_red = center_dest.red() - right_dest.red();

            double upgrad_orig_blue = center_orig.blue() - up_orig.blue();
            double upgrad_dest_blue = center_dest.blue() - up_dest.blue();

            double downgrad_orig_blue = center_orig.blue() - down_orig.blue();
            double downgrad_dest_blue = center_dest.blue() - down_dest.blue();
            
            double leftgrad_orig_blue = center_orig.blue() - left_orig.blue();
            double leftgrad_dest_blue = center_dest.blue() - left_dest.blue();
                        
            double rightgrad_orig_blue = center_orig.blue() - right_orig.blue();
            double rightgrad_dest_blue = center_dest.blue() - right_dest.blue();

            double upgrad_orig_green = center_orig.green() - up_orig.green();
            double upgrad_dest_green = center_dest.green() - up_dest.green();

            double downgrad_orig_green = center_orig.green() - down_orig.green();
            double downgrad_dest_green = center_dest.green() - down_dest.green();
            
            double leftgrad_orig_green = center_orig.green() - left_orig.green();
            double leftgrad_dest_green = center_dest.green() - left_dest.green();
                        
            double rightgrad_orig_green = center_orig.green() - right_orig.green();
            double rightgrad_dest_green = center_dest.green() - right_dest.green();

#define ABSMAX(a, b) (( (( (a) >= 0 ? (a) : (-a) )) > ((  (b) >= 0 ? (b) : (-b)   )) ? ((a)) : ((b)) ))
#define ABS(a) (( (a) >= 0 ? (a) : -(a) ))

            // if (ABS(upgrad_orig_red) + ABS(upgrad_orig_green) + ABS(upgrad_orig_blue) +
            //     ABS(downgrad_orig_red) + ABS(downgrad_orig_green) + ABS(downgrad_orig_blue) + 
            //     ABS(leftgrad_orig_red) + ABS(leftgrad_orig_green) + ABS(leftgrad_orig_blue) + 
            //     ABS(rightgrad_orig_red) + ABS(rightgrad_orig_green) + ABS(rightgrad_orig_blue) > 
            //     ABS(upgrad_dest_red) + ABS(upgrad_dest_green) + ABS(upgrad_dest_blue) +
            //     ABS(downgrad_dest_red) + ABS(downgrad_dest_green) + ABS(downgrad_dest_blue) + 
            //     ABS(leftgrad_dest_red) + ABS(leftgrad_dest_green) + ABS(leftgrad_dest_blue) + 
            //     ABS(rightgrad_dest_red) + ABS(rightgrad_dest_green) + ABS(rightgrad_dest_blue) ) {

            //     b_vec_r(current_row) += upgrad_orig_red;
            //     b_vec_g(current_row) += upgrad_orig_green;
            //     b_vec_b(current_row) += upgrad_orig_blue;

            //     b_vec_r(current_row) += downgrad_orig_red;
            //     b_vec_g(current_row) += downgrad_orig_green;
            //     b_vec_b(current_row) += downgrad_orig_blue;

            //     b_vec_r(current_row) += leftgrad_orig_red;
            //     b_vec_g(current_row) += leftgrad_orig_green;
            //     b_vec_b(current_row) += leftgrad_orig_blue;

            //     b_vec_r(current_row) += rightgrad_orig_red;
            //     b_vec_g(current_row) += rightgrad_orig_green;
            //     b_vec_b(current_row) += rightgrad_orig_blue;


            // } else {
            //     b_vec_r(current_row) += upgrad_dest_red;
            //     b_vec_g(current_row) += upgrad_dest_green;
            //     b_vec_b(current_row) += upgrad_dest_blue;

            //     b_vec_r(current_row) += downgrad_dest_red;
            //     b_vec_g(current_row) += downgrad_dest_green;
            //     b_vec_b(current_row) += downgrad_dest_blue;

            //     b_vec_r(current_row) += leftgrad_dest_red;
            //     b_vec_g(current_row) += leftgrad_dest_green;
            //     b_vec_b(current_row) += leftgrad_dest_blue;

            //     b_vec_r(current_row) += rightgrad_dest_red;
            //     b_vec_g(current_row) += rightgrad_dest_green;
            //     b_vec_b(current_row) += rightgrad_dest_blue;

            // }

            // if (ABS(upgrad_orig_red) + ABS(upgrad_orig_green) + ABS(upgrad_orig_blue) > 
            //     ABS(upgrad_dest_red) + ABS(upgrad_dest_green) + ABS(upgrad_dest_blue)) {
            //     b_vec_r(current_row) += upgrad_orig_red;
            //     b_vec_g(current_row) += upgrad_orig_green;
            //     b_vec_b(current_row) += upgrad_orig_blue;
            // } else {
            //     b_vec_r(current_row) += upgrad_dest_red;
            //     b_vec_g(current_row) += upgrad_dest_green;
            //     b_vec_b(current_row) += upgrad_dest_blue;
            // }

            // if (ABS(downgrad_orig_red) + ABS(downgrad_orig_green) + ABS(downgrad_orig_blue) > 
            //     ABS(downgrad_dest_red) + ABS(downgrad_dest_green) + ABS(downgrad_dest_blue)) {
            //     b_vec_r(current_row) += downgrad_orig_red;
            //     b_vec_g(current_row) += downgrad_orig_green;
            //     b_vec_b(current_row) += downgrad_orig_blue;
            // } else {
            //     b_vec_r(current_row) += downgrad_dest_red;
            //     b_vec_g(current_row) += downgrad_dest_green;
            //     b_vec_b(current_row) += downgrad_dest_blue;
            // }

            // if (ABS(leftgrad_orig_red) + ABS(leftgrad_orig_green) + ABS(leftgrad_orig_blue) > 
            //     ABS(leftgrad_dest_red) + ABS(leftgrad_dest_green) + ABS(leftgrad_dest_blue)) {
            //     b_vec_r(current_row) += leftgrad_orig_red;
            //     b_vec_g(current_row) += leftgrad_orig_green;
            //     b_vec_b(current_row) += leftgrad_orig_blue;
            // } else {
            //     b_vec_r(current_row) += leftgrad_dest_red;
            //     b_vec_g(current_row) += leftgrad_dest_green;
            //     b_vec_b(current_row) += leftgrad_dest_blue;
            // }

            // if (ABS(rightgrad_orig_red) + ABS(rightgrad_orig_green) + ABS(rightgrad_orig_blue) > 
            //     ABS(rightgrad_dest_red) + ABS(rightgrad_dest_green) + ABS(rightgrad_dest_blue)) {
            //     b_vec_r(current_row) += rightgrad_orig_red;
            //     b_vec_g(current_row) += rightgrad_orig_green;
            //     b_vec_b(current_row) += rightgrad_orig_blue;
            // } else {
            //     b_vec_r(current_row) += rightgrad_dest_red;
            //     b_vec_g(current_row) += rightgrad_dest_green;
            //     b_vec_b(current_row) += rightgrad_dest_blue;
            // }

            b_vec_r(current_row) += ABSMAX(upgrad_orig_red, upgrad_dest_red);
            b_vec_g(current_row) += ABSMAX(upgrad_orig_green, upgrad_dest_green);
            b_vec_b(current_row) += ABSMAX(upgrad_orig_blue, upgrad_dest_blue);

            b_vec_r(current_row) += ABSMAX(downgrad_orig_red, downgrad_dest_red);
            b_vec_g(current_row) += ABSMAX(downgrad_orig_green, downgrad_dest_green);
            b_vec_b(current_row) += ABSMAX(downgrad_orig_blue, downgrad_dest_blue);

            b_vec_r(current_row) += ABSMAX(leftgrad_orig_red, leftgrad_dest_red);
            b_vec_g(current_row) += ABSMAX(leftgrad_orig_green, leftgrad_dest_green);
            b_vec_b(current_row) += ABSMAX(leftgrad_orig_blue, leftgrad_dest_blue);

            b_vec_r(current_row) += ABSMAX(rightgrad_orig_red, rightgrad_dest_red);
            b_vec_g(current_row) += ABSMAX(rightgrad_orig_green, rightgrad_dest_green);
            b_vec_b(current_row) += ABSMAX(rightgrad_orig_blue, rightgrad_dest_blue);

            // b_vec_r(current_row) += upgrad_orig_red;
            // b_vec_g(current_row) += upgrad_orig_green;
            // b_vec_b(current_row) += upgrad_orig_blue;

            // b_vec_r(current_row) += downgrad_orig_red;
            // b_vec_g(current_row) += downgrad_orig_green;
            // b_vec_b(current_row) += downgrad_orig_blue;

            // b_vec_r(current_row) += leftgrad_orig_red;
            // b_vec_g(current_row) += leftgrad_orig_green;
            // b_vec_b(current_row) += leftgrad_orig_blue;

            // b_vec_r(current_row) += rightgrad_orig_red;
            // b_vec_g(current_row) += rightgrad_orig_green;
            // b_vec_b(current_row) += rightgrad_orig_blue;

            // b_vec_r(current_row) += upgrad_dest_red;
            // b_vec_g(current_row) += upgrad_dest_green;
            // b_vec_b(current_row) += upgrad_dest_blue;

            // b_vec_r(current_row) += downgrad_dest_red;
            // b_vec_g(current_row) += downgrad_dest_green;
            // b_vec_b(current_row) += downgrad_dest_blue;

            // b_vec_r(current_row) += leftgrad_dest_red;
            // b_vec_g(current_row) += leftgrad_dest_green;
            // b_vec_b(current_row) += leftgrad_dest_blue;

            // b_vec_r(current_row) += rightgrad_dest_red;
            // b_vec_g(current_row) += rightgrad_dest_green;
            // b_vec_b(current_row) += rightgrad_dest_blue;


            // ------- Ugly pieces end --------

            // Bounaries
            // Up
            if (IS_EXTERIOR(i - 1, j)) {
                b_vec_r(current_row) += up_dest.red();
                b_vec_g(current_row) += up_dest.green();
                b_vec_b(current_row) += up_dest.blue();
            }

            // Down
            if (IS_EXTERIOR(i + 1, j)) {
                b_vec_r(current_row) += down_dest.red();
                b_vec_g(current_row) += down_dest.green();
                b_vec_b(current_row) += down_dest.blue();
            }

            // Left
            if (IS_EXTERIOR(i, j - 1)) {
                b_vec_r(current_row) += left_dest.red();
                b_vec_g(current_row) += left_dest.green();
                b_vec_b(current_row) += left_dest.blue();
            }

            // Right
            if (IS_EXTERIOR(i, j + 1)) {
                b_vec_r(current_row) += right_dest.red();
                b_vec_g(current_row) += right_dest.green();
                b_vec_b(current_row) += right_dest.blue();
            }
        }
    }

    printf("b vector dest term build complete\n");

    // Solve the equation
    Eigen::VectorXd result_r_vec = solver.solve(b_vec_r);
    std::cout << "#iterations:     " << solver.iterations() << std::endl;
    std::cout << "estimated error: " << solver.error()      << std::endl;
    Eigen::VectorXd result_g_vec = solver.solve(b_vec_g);
    std::cout << "#iterations:     " << solver.iterations() << std::endl;
    std::cout << "estimated error: " << solver.error()      << std::endl;
    Eigen::VectorXd result_b_vec = solver.solve(b_vec_b);
    std::cout << "#iterations:     " << solver.iterations() << std::endl;
    std::cout << "estimated error: " << solver.error()      << std::endl;

    printf("equation solving complete\n");

    // Assign the image, at this point it's allocated but uninitialized
    result = QImage(orig.width(), orig.height(), orig.format());

    int row_count = 0;
    // Copy the top border
    for (int m = 0; m < orig.width(); m++) {
        result.setPixelColor(m, 0, dest.pixelColor(m, 0));
    }

#define TRUNCATE(x) (( x >= 255 ? 255 : (x <= 0 ? 0 : x)))
// #define TRUNCATE(x) (( (int)ABS(x/100) % 256 ))

    for (int i = 1; i < orig.height() - 1; i++) {
        result.setPixelColor(0, i, dest.pixelColor(0, i));
        for (int j = 1; j < orig.width() - 1; j++) {
            QColor dest_color(
                TRUNCATE(result_r_vec(row_count)),
                TRUNCATE(result_g_vec(row_count)),
                TRUNCATE(result_b_vec(row_count)));
            result.setPixelColor(j, i, dest_color);
            row_count++;
        }
        result.setPixelColor(orig.width() - 1, i, dest.pixelColor(orig.width() - 1, i));
    }

    // Copy the bottom border
    for (int m = 0; m < orig.width(); m++) {
        result.setPixelColor(m, orig.height() - 1, dest.pixelColor(m, orig.height() - 1));
    }

    printf("picture build complete\n");

    return result;
}