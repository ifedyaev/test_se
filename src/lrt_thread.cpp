#include "lrt_thread.hpp"
#include <QImage>
#include <cmath>
#include <math.h>
#include <QDir>
#include <QDebug>
#include <iostream>

static constexpr float C_ANG_TO_RAD = M_PI/180.0f;
static constexpr float C_EPS = std::numeric_limits<float>::epsilon()*10;
static constexpr QImage::Format C_FRGB32 = QImage::Format_RGB32;

LRTThread::LRTThread(const std::shared_ptr<QImage>& image) : QThread()
{
    m_image_input = image;
    setup_array_k();
    setup_array_axis_xy();
    /* init bound */
    m_lb_y = 0.0f;
    m_ub_y = m_image_input->height() - 1;
}

void LRTThread::run()
{
    m_is_run = true;
    const int32_t n_y = m_image_input->height();
    const int32_t n_x = m_image_input->width();
    const int32_t n_k = static_cast<int32_t>(m_arr_k.size());

    const int32_t half_size_w = static_cast<int32_t>( n_x/2.0f + 0.5f );

    m_image_output = std::make_shared<QImage>(/* width  = */n_k,
                                              /* height = */n_y,
                                              /* format = */C_FRGB32);

    /* ============= create matrix ===========
     * -> fill
     * -> noralization by bw
     * =======================================
     */
    std::vector<float> buffer;
    std::vector<float*> matrix;
    init_matrix(/* buffer = */buffer,/* matrix = */matrix,
                /* n_k = */n_k,/* n_y = */ n_y);

    int32_t iy; /* loop index height */
    int32_t ik; /* loop index angle -> k line */

    /* === middle matrix */
    int32_t ik_lim = -1;
    for(ik = 0; ik < n_k; ++ik){
        if(m_arr_k[ik] == 0.0f){
            ik_lim = ik;
        }
    }

    if(ik_lim != -1 and n_x%2 != 0){
        ik = ik_lim;
        const int32_t mid_x = n_x/2;
        float mid_sum{0.0f};
        for(iy = 0; iy < n_y; ++iy){
            mid_sum += static_cast<float>(m_image_input->pixelColor(mid_x,iy).red());
        }
        /* fill */
        for(iy = 0; iy < n_y; ++iy){
            matrix[ik][iy] = mid_sum;
        }
    }
    else{
        ik_lim = -1;
    }

    const float step_proc = 100.0f/(n_y - 1);
    /* loop Oy -> height */
    for(iy = 0; iy < n_y; ++iy){
        emit updata_progress_bar(step_proc*iy);
        /* set progress bar */
        const float cur_b = m_arr_y[iy]; /* b */
        /* loop angle -> k*/
        for(ik = 0; ik < n_k; ++ik){
            if(ik == ik_lim){ continue; }
            const float cur_k = m_arr_k[ik];
            /* left */
            compute_left_matrix(/* cur_k = */cur_k,/* cur_b = */cur_b,
                                /* ik = */ik,/* iy = */iy,
                                /* half_size_w = */half_size_w,
                                /* matrix = */matrix);
            /* right */
            compute_right_matrix(/* cur_k = */cur_k,/* cur_b = */cur_b,
                                 /* ik = */ik,/* iy = */iy,
                                 /* half_size_w = */half_size_w,/* n_x = */n_x,
                                 /* matrix = */matrix);

            if(not m_is_run) { break; }
        }/* end loop angle */
        if(not m_is_run) { break; }

    }/* end loop height*/

    if(m_is_run){
        set_gray_image_by_data(/* buffer = */buffer,/* matrix = */matrix,
                               /* n_k = */n_k,/* n_y = */n_y);
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
    /* fill height coord -> Oy */
    for(i = 0; i < height; ++i){
        m_arr_y[i] = i;
    }
    return;
}

float LRTThread::inperpolation_y(const float x_out,const int32_t idx_x) const
{
    /* left interp data */
    const float x_lhs = static_cast<float>(static_cast<int32_t>(x_out));
    const float y_lhs = static_cast<float>(m_image_input->pixelColor(idx_x,x_lhs).red());
    /* right intepr data */

    const int32_t fx_rhs = x_out + 1.0f;
    float x_rhs;
    if(fx_rhs - m_ub_y > C_EPS){
        x_rhs = static_cast<float>(static_cast<int32_t>(fx_rhs - 2.0f));
    }
    else{
        x_rhs = static_cast<float>(static_cast<int32_t>(fx_rhs));
    }
    const float y_rhs = static_cast<float>(m_image_input->pixelColor(idx_x,x_rhs).red());

    const float y_out = line_interp(/* x_lhs = */x_lhs,/* y_lhs = */y_lhs,
                                    /* x_rhs = */x_rhs,/* y_rhs = */y_rhs,
                                    /* x_out = */x_out);
    return y_out;
}

void LRTThread::compute_left_matrix(const float cur_k,const float cur_b,
                                    const int32_t ik,const int32_t iy,
                                    const int32_t half_size_w,
                                    std::vector<float*>& matrix)
{
    int32_t ix;
    /* loop Ox -> left */
    for(ix = half_size_w - 1; ix >= 0; --ix){
        const float cur_x = m_arr_x[ix];
        const float cur_y = cur_k*cur_x + cur_b;
        /* check lower bound */
        if(cur_y != 0.0f){
            if(cur_y - m_lb_y < C_EPS){ break; }
            if(cur_y - m_ub_y > C_EPS){ break; }
        }
        /* inteprolation data */
        matrix[ik][iy] += inperpolation_y(/* x_out = */cur_y,/* idx_x = */ix);
    }/* end loop left */
    return;
}

void LRTThread::compute_right_matrix(const float cur_k, const float cur_b,
                                     const int32_t ik, const int32_t iy,
                                     const int32_t half_size_w, const int32_t n_x,
                                     std::vector<float *> &matrix)
{
    int32_t ix;

    /* loop Ox -> right */
    for(ix = half_size_w; ix < n_x; ++ix){
        const float cur_x = m_arr_x[ix];
        const float cur_y = cur_k*cur_x + cur_b;
        /* check lower bound */
        if(cur_y != 0.0f){
            if(cur_y - m_lb_y < C_EPS){ break; }
            if(cur_y - m_ub_y > C_EPS){ break; }
        }
        /* inteprolation data */
        matrix[ik][iy] += inperpolation_y(/* x_out = */cur_y,/* idx_x = */ix);
    }/* end loop right */
    return;
}

void LRTThread::set_gray_image_by_data(const std::vector<float> &buffer, const std::vector<float *> &matrix,const int32_t n_k,const int32_t n_y)
{
    const int32_t idx_max    = find_imax(buffer);
    const float scale_matrix = 255.0f/buffer[idx_max];
    /* loop set gray in image */
    /* loop heigth */
    int32_t ik;
    int32_t iy;
    for(ik = 0; ik < n_k; ++ik){
        /* loop width */
        for(iy = 0; iy < n_y; ++iy){
            const float value = matrix[ik][iy];
            int32_t gray{0};
            if(value != 0.0f){
                gray = static_cast<int32_t>(value*scale_matrix);
            }
            m_image_output->setPixelColor(ik,iy,qRgb(gray,gray,gray));
        }/* end loop width*/
    }/* end loop heigth */
    return;
}

void LRTThread::init_matrix(std::vector<float>& buffer, std::vector<float *>& matrix, const int32_t n_k, const int32_t n_y)
{
    buffer.resize(n_k*n_y);
    std::fill(buffer.begin(),buffer.end(),0.0f);

    matrix.resize(n_k);
    int32_t i;
    /* fill pointer */
    for(i = 0; i < n_k; ++i){
        matrix[i] = buffer.data() + i*n_y;
    }
    return;
}

int32_t LRTThread::find_imax(const std::vector<float> &data)
{
    /* check size */
    if(data.size() == 0){ return 0;}
    int32_t idx_max;
    float val_max = std::numeric_limits<float>::lowest();
    const int32_t n_data = static_cast<int32_t>(data.size());

    int32_t i;
    for(i = 0; i < n_data; ++i){
        const float value = data[i];
        if(val_max - value < C_EPS){
            val_max = value;
            idx_max = i;
        }
    }
    return idx_max;
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
