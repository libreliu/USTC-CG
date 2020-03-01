#include "IDWWarper.h"

#define DIST_2(x1, y1, x2, y2) (((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1)))

// the static instance
IDWWarper IDWWarper::inst;

IDWWarper *IDWWarper::getInstance() {
    return &(inst);
}

const IDWWarper::IntPoint& IDWWarper::doTrans(const IDWWarper::IntPoint &p) {
    double px = p.getX();
    double py = p.getY();

    double fp_x = 0;
    double fp_y = 0;

    double sigma_sum = 0;
    for (int i = 0; i < total_pairs; i++) {
        double pix = ctrl_point_pair[i].first.getX();
        double piy = ctrl_point_pair[i].first.getY();
        sigma_sum += 1.0 / DIST_2(px, py, pix, piy);
    }

    // f_i(p)
    for (int i = 0; i < total_pairs; i++) {
        double pix = ctrl_point_pair[i].first.getX();
        double piy = ctrl_point_pair[i].first.getY();

        double t11 = t_mat[i].t11;
        double t12 = t_mat[i].t12;
        double t21 = t_mat[i].t21;
        double t22 = t_mat[i].t22;

        double qix = ctrl_point_pair[i].second.getX();
        double qiy = ctrl_point_pair[i].second.getY();

        double fip_x = qix + t11 * (px - pix) + t12 * (py - piy);
        double fip_y = qiy + t21 * (px - pix) + t22 * (py - piy);

        double wi_p = (1.0 / DIST_2(px, py, pix, piy)) / sigma_sum;

        fp_x += wi_p * fip_x;
        fp_y += wi_p * fip_y;
    }

    static IntPoint ret;
    ret.setX((int)fp_x).setY((int)fp_y);

    return ret;
}

// Note: need order here!
void IDWWarper::initialize(std::vector<IntMapPoint> m) {

    total_pairs = m.size();

    // reserve enough memory
    t_mat.clear();
    t_mat = std::vector<TMatrix>(total_pairs);

    // aux info
    ctrl_point_pair = m;

    // calculate T matrices
    for (int i = 0; i < total_pairs; i++) {

        /* Eqn:
        * a*t11+b*t12 = free_1
        * c*t11+d*t12 = free_2
        * e*t21+f*t22 = free_3
        * g*t21+h*t22 = free_4
        */
        double a = 0, b = 0, c = 0, d = 0, e = 0, f = 0, g = 0, h = 0;
        double free_1 = 0, free_2 = 0, free_3 = 0, free_4 = 0;

        double t11, t12, t21, t22;

        for (int j = 0; j < total_pairs; j++) {
            if (j == i) {
                continue;
            }

            double pjx = m[j].first.getX();
            double pix = m[i].first.getX();
            double pjy = m[j].first.getY();
            double piy = m[i].first.getY();

            double qjx = m[j].second.getX();
            double qix = m[i].second.getX();
            double qjy = m[j].second.getY();
            double qiy = m[i].second.getY();

            double pjx_sub_pix = pjx - pix;
            double pjy_sub_piy = pjy - piy;
            double qjx_sub_qix = qjx - qix;
            double qjy_sub_qiy = qjy - qiy;

            double sigma_i_pj = 1.0 / DIST_2(pjx, pjy, pix, piy);


            // need atomicity here
            a += 2 * sigma_i_pj * pjx_sub_pix * pjx_sub_pix;
            b += 2 * sigma_i_pj * pjx_sub_pix * pjy_sub_piy;
            // free_1 += -2 * sigma_i_pj * pjx_sub_pix * qjx_sub_qix;
            free_1 += 2 * sigma_i_pj * pjx_sub_pix * qjx_sub_qix;

            c += 2 * sigma_i_pj * pjx_sub_pix * pjy_sub_piy;
            d += 2 * sigma_i_pj * pjy_sub_piy * pjy_sub_piy;
            // free_2 += -2 * sigma_i_pj * qjx_sub_qix * pjy_sub_piy;
            free_2 += 2 * sigma_i_pj * qjx_sub_qix * pjy_sub_piy;

            e += 2 * sigma_i_pj * pjx_sub_pix * pjx_sub_pix;
            f += 2 * sigma_i_pj * pjx_sub_pix * pjy_sub_piy;
            // free_3 += -2 * sigma_i_pj * pjx_sub_pix * qjy_sub_qiy;
            free_3 += 2 * sigma_i_pj * pjx_sub_pix * qjy_sub_qiy;

            g += 2 * sigma_i_pj * pjx_sub_pix * pjy_sub_piy;
            h += 2 * sigma_i_pj * pjy_sub_piy * pjy_sub_piy;
            // free_4 += -2 * sigma_i_pj * pjy_sub_piy * qjy_sub_qiy;
            free_4 += 2 * sigma_i_pj * pjy_sub_piy * qjy_sub_qiy;
        }

        // solve the equation
        t11 = (free_2 * b - free_1 * d) / (b * c - a * d);
        t12 = (free_1 * c - free_2 * a) / (b * c - a * d);
        t21 = (free_4 * f - free_3 * h) / (g * f - e * h);
        t22 = (free_3 * g - free_4 * e) / (g * f - e * h);
        /* Eqn:
        * a*t11+b*t12 = free_1
        * c*t11+d*t12 = free_2
        * e*t21+f*t22 = free_3
        * g*t21+h*t22 = free_4
        */

        // store
        TMatrix t;
        t.t11 = t11;
        t.t12 = t12;
        t.t21 = t21;
        t.t22 = t22;

        printf("i=%d, t11=%lf, t12=%lf, t21=%lf, t22=%lf\n", i, t11, t12, t21, t22);
        // validate
        printf("1: %lf = %lf\n", a * t11 + b * t12, free_1);
        printf("2: %lf = %lf\n", c * t11 + d * t12, free_2);
        printf("3: %lf = %lf\n", e * t21 + f * t22, free_3);
        printf("4: %lf = %lf\n", g * t21 + h * t22, free_4);

        t_mat[i] = t;
    }
}