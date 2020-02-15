#include "lrt_thread.hpp"
#include <QImage>
#include <cmath>
#include <math.h>
#include <QDir>
#include <QDebug>
#include <iostream>

#include "matrix.hpp"

static constexpr float C_ANG_TO_RAD = M_PI/180.0f;
static constexpr float C_EPS = std::numeric_limits<float>::epsilon()*10;
static const QImage::Format C_FRGB32 = QImage::Format_RGB32;

LRTThread::LRTThread(const std::shared_ptr<QImage>& image) : QThread()
{
    m_image_input = image;
    setup_array_k();
    setup_array_axis_xy();
}

void LRTThread::run()
{
    m_is_run = true;
    const int32_t n_row = m_image_input->height();
    const int32_t n_x = m_image_input->width();
    const int32_t n_col = static_cast<int32_t>(m_arr_k.size());

    const int32_t half_n_x = static_cast<int32_t>( n_x/2.0f );

    m_image_output = std::make_shared<QImage>(/* width  = */n_col,
                                              /* height = */n_row,
                                              /* format = */C_FRGB32);

    /***********************************
     *          create matrix
     * -> compute Line Radon Transform
     * -> count sum point
     ***********************************
     */
    Matrix<float> matrix(/* n_col = */n_col,/* n_row = */n_row);
    Matrix<int32_t> count_matrix(/* n_col = */n_col,/* n_row = */n_row);

    int32_t row; /* loop index height */
    int32_t col; /* loop index angle -> k line */

    const int32_t ic_lim = compute_k_zeros(/* matrix = */matrix,
                                           /* count_matrix = */count_matrix,
                                           /* n_col = */n_col,/* n_row = */n_row,
                                           /* n_x = */n_x);

    const float step_proc = 100.0f/(n_row - 1);
    /* loop Oy -> height */
    for(row = 0; row < n_row; ++row){
        emit updata_progress_bar(step_proc*row);
        /* set progress bar */
        const float cur_b = m_arr_y[row]; /* b */
        /* loop angle -> k*/
        for(col = 0; col < n_col; ++col){
            if(col == ic_lim){ continue; }
            const float cur_k = m_arr_k[col];
            /* left */
            compute_left_matrix(/* cur_k = */cur_k,/* cur_b = */cur_b,
                                /* col = */col,/* row = */row,
                                /* half_n_x = */half_n_x,
                                /* matrix = */matrix,
                                /* count_matrix = */count_matrix);
            /* right */
            compute_right_matrix(/* cur_k = */cur_k,/* cur_b = */cur_b,
                                 /* col = */col,/* row = */row,
                                 /* half_n_x = */half_n_x,/* n_x = */n_x,
                                 /* matrix = */matrix,
                                 /* count_matrix = */count_matrix);

            if(not m_is_run) { break; }
        }/* end loop angle */
        if(not m_is_run) { break; }

    }/* end loop height*/

    if(m_is_run){
        set_gray_image_by_data(/* matrix = */matrix,
                               /* count_matrix = */count_matrix,
                               /* n_col = */n_col,/* n_row = */n_row);
        emit end_of_job(m_image_output);
    }
    m_is_run = false;
    return;
}

void LRTThread::setup_array_k()
{
    constexpr float beg_angle = 0.0f;
    constexpr float end_angle = 180.0f;
    constexpr int32_t n_angle = 180;

    constexpr float step_angle = (end_angle - beg_angle)/n_angle;

    m_arr_k.resize(n_angle);
    int32_t i; /* loop index */
    /* loop fill angle */
    for(i = 0; i < n_angle; ++i){
        const float cur_angle = beg_angle + step_angle*i;
        if( std::abs(cur_angle - 90.0f) < C_EPS ){
            m_arr_k[i] = 0.0f;
        }
        else{
            m_arr_k[i] = std::tan(cur_angle*C_ANG_TO_RAD);
        }
    }
    return;
}

void LRTThread::setup_array_axis_xy()
{
    int32_t i;/* loop index */

    /* Ox */
    const int32_t width  = m_image_input->width();
    m_arr_x.resize(width);
    const float half_width = (width - 1)/2.0f;
    /* fill width coord -> Ox */
    for(i = 0; i < width; ++i){
        m_arr_x[i] = -half_width + i;
    }

    /* Oy */
    const int32_t height = m_image_input->height();
    m_arr_y.resize(height);
    const float half_height = (height - 1)/2.0f;
    /* fill height coord -> Oy */
    for(i = 0; i < height; ++i){
        m_arr_y[i] = -half_height + i;
    }

    /* init bound */
    m_lb_y = -half_height;
    m_ub_y = half_height;
    return;
}

float LRTThread::inperpolation_y(const float row_out,const int32_t col) const
{
    /* left interp data */
    const float row_lhs   = static_cast<float>(static_cast<int32_t>(row_out + m_ub_y));
    const float color_lhs = static_cast<float>(m_image_input->pixelColor(col,row_lhs).red());
    /* right intepr data */

    float row_rhs = static_cast<float>(static_cast<int32_t>(row_out + 1.0f + m_ub_y));
    if(row_rhs - m_ub_y > C_EPS){
        row_rhs = static_cast<float>(static_cast<int32_t>(row_rhs - 2.0f));
    }
    const float color_rhs = static_cast<float>(m_image_input->pixelColor(col,row_rhs).red());

    const float color_out = line_interp(/* x_lhs = */row_lhs,/* y_lhs = */color_lhs,
                                        /* x_rhs = */row_rhs,/* y_rhs = */color_rhs,
                                        /* x_out = */row_out);
    return color_out;
}

void LRTThread::compute_left_matrix(const float cur_k,const float cur_b,
                                    const int32_t col,const int32_t row,
                                    const int32_t half_n_x,
                                    Matrix<float>& matrix,
                                    Matrix<int32_t>& count_matrix)
{
    int32_t ix;
    /* loop Ox -> left */
    for(ix = half_n_x - 1; ix >= 0; --ix){
        const float cur_x = m_arr_x[ix];
        const float cur_y = cur_k*cur_x + cur_b;
        /* check lower bound */
        if(cur_y != 0.0f){
            if(cur_y - m_lb_y < C_EPS){ break; }
            if(cur_y - m_ub_y > C_EPS){ break; }
        }
        /* inteprolation data */
        matrix[col][row]       += inperpolation_y(/* row_out = */cur_y,/* col = */ix);
        count_matrix[col][row] += 1;
    }/* end loop left */
    return;
}

void LRTThread::compute_right_matrix(const float cur_k, const float cur_b,
                                     const int32_t col, const int32_t row,
                                     const int32_t half_n_x, const int32_t n_x,
                                     Matrix<float>& matrix,
                                     Matrix<int32_t>& count_matrix)
{
    int32_t ix;

    /* loop Ox -> right */
    for(ix = half_n_x; ix < n_x; ++ix){
        const float cur_x = m_arr_x[ix];
        const float cur_y = cur_k*cur_x + cur_b;
        /* check lower bound */
        if(cur_y != 0.0f){
            if(cur_y - m_lb_y < C_EPS){ break; }
            if(cur_y - m_ub_y > C_EPS){ break; }
        }
        /* inteprolation data */
        matrix[col][row]       += inperpolation_y(/* row_out = */cur_y,/* col = */ix);
        count_matrix[col][row] += 1;
    }/* end loop right */
    return;
}

void LRTThread::set_gray_image_by_data(Matrix<float>& matrix,
                                       Matrix<int32_t>& count_matrix,
                                       const int32_t n_col,const int32_t n_row)
{
    int32_t col;
    int32_t row;
    for(col = 0; col < n_col; ++col){
        /* loop width */
        for(row = 0; row < n_row; ++row){
            float scale = 1.0f;
            const int32_t n = count_matrix[col][row];
            if(n != 0){
                scale = 1.0f/n;
            }
            matrix[col][row] *= scale;
        }/* end loop width*/
    }/* end loop heigth */
    matrix.normalization_on_gray();
    /* loop set gray in image */
    /* loop heigth */

    for(col = 0; col < n_col; ++col){
        /* loop width */
        for(row = 0; row < n_row; ++row){
            const int32_t gray = static_cast<int32_t>(matrix[col][row]);
            m_image_output->setPixelColor(col,row,qRgb(gray,gray,gray));
        }/* end loop width*/
    }/* end loop heigth */
    return;
}

int32_t LRTThread::compute_k_zeros(Matrix<float> &matrix,
                                   Matrix<int32_t> &count_matrix,
                                   const int32_t n_col, const int32_t n_row,
                                   const int32_t n_x)
{
    int32_t row; /* loop index height */
    int32_t col; /* loop index angle -> k line */

    /* === middle matrix */
    int32_t ic_lim = -1;
    for(col = 0; col < n_col; ++col){
        if(m_arr_k[col] == 0.0f){
            ic_lim = col;
        }
    }

    if(ic_lim != -1 and n_x%2 != 0){
        col = ic_lim;
        const int32_t mid_x = n_x/2;
        float mid_sum{0.0f};
        for(row = 0; row < n_row; ++row){
            mid_sum += static_cast<float>(m_image_input->pixelColor(mid_x,row).red());
        }
        /* fill */
        for(row = 0; row < n_row; ++row){
            matrix[col][row]       = mid_sum;
            count_matrix[col][row] = n_row;
        }
    }
    else{
        ic_lim = -1;
    }
    return ic_lim;
}

float LRTThread::line_interp(const float x_lhs, const float y_lhs,
                             const float x_rhs, const float y_rhs,
                             const float x_out) const noexcept
{
    /*
     * Y ^                 y_rhs
     *   |        y_out     *
     *   | y_lhs    *       |
     *   |   *      |       |
     *   |   |      |       |
     *   |   |      |       |
     *   ----*------*-------*-------> X
     *     x_lhs   x_out   x_rhs
     */
    const float cur_x = (x_out - x_lhs)/(x_rhs - x_lhs);
    return (y_lhs + cur_x*(y_rhs - y_lhs));
}
