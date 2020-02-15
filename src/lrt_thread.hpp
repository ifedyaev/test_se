#ifndef LRT_THREAD_HPP
#define LRT_THREAD_HPP
/***********************************************
 *               class LRTThread
 * The class was created to perform
 * a Linear Radon Transformation in
 * a separate Thread
 ***********************************************
 */
#include <QThread>
#include <memory>

template<typename T> class Matrix;

class LRTThread : public QThread
{
public:
    explicit LRTThread(const std::shared_ptr<QImage>& image);
    void stop() { m_is_run = false; return; }

private:
    Q_OBJECT
    void run() override;

private:
    bool m_is_run{false};/* flag start-stop */

    std::shared_ptr< QImage > m_image_input{nullptr};   /* input image */
    std::shared_ptr< QImage > m_image_output{nullptr};  /* output image */

    /* radon data */
    std::vector<float> m_arr_k;
    /* axis x y */
    std::vector<float> m_arr_x;
    std::vector<float> m_arr_y;
    /* border */
    float m_ub_y{0.0f}; /* upper bound */
    float m_lb_y{0.0f}; /* lower bound */

    /**
     * @brief setup_array_k angle -> to k == tan
     */
    void setup_array_k();

    /**
     * @brief setup_array_axis_xy create Ox / Oy coodinate system
     */
    void setup_array_axis_xy();

    /**
     * @brief inperpolation_row compute all point interpolation and do line interplation
     * @param row_out   row matrix
     * @param col       column matrix
     * @return
     */
    float inperpolation_y(const float row_out,const int32_t col) const;

    /**
     * @brief compute_left_matrix compute left matrix
     * relative to the center of the coordinate system
     * -> if one compyte Y out of coordinate matrix ( break compute )
     * @param cur_k     k line
     * @param cur_b     b line
     * @param col       column matrix
     * @param row       row matrix
     * @param half_n_x  half size x coordinate
     * @param matrix       matrix sum
     * @param count_matrix matrix count point in one [row][col] sum
     */
    void compute_left_matrix(const float cur_k, const float cur_b,
                             const int32_t col, const int32_t row,
                             const int32_t half_n_x,
                             Matrix<float>& matrix,
                             Matrix<int32_t>& count_matrix);

    /**
     * @brief compute_right_matrix Compute right matrix
     * relative to the center of the coordinate system
     * -> if one compyte Y out of coordinate matrix ( break compute )
     * @param cur_k     k line
     * @param cur_b     b line
     * @param col       column matrix
     * @param row       row matrix
     * @param half_n_x  half size x coordinate
     * @param n_x       size x
     * @param matrix       matrix sum
     * @param count_matrix matrix count point in one [row][col] sum
     */
    void compute_right_matrix(const float cur_k, const float cur_b,
                              const int32_t col, const int32_t row,
                              const int32_t half_n_x, const int32_t n_x,
                              Matrix<float>& matrix,
                              Matrix<int32_t>& count_matrix);

    /**
     * @brief set_gray_image_by_data
     * @param matrix       matrix sum
     * @param count_matrix matrix count point in one [row][col] sum
     * @param n_col     size column
     * @param n_row     size row
     */
    void set_gray_image_by_data(Matrix<float>& matrix,
                                Matrix<int32_t>& count_matrix,
                                const int32_t n_col,const int32_t n_row);

    int32_t compute_k_zeros(Matrix<float>& matrix,
                            Matrix<int32_t>& count_matrix,
                            const int32_t n_col,const int32_t n_row,
                            const int32_t n_x);

    /**
     * @brief line_interp line interp
     * @param x_lhs x coordinate x left
     * @param y_lhs y coordinate y left
     * @param x_rhs x coordinate x right
     * @param y_rhs y coordinate y right
     * @param x_out x coordinate x output
     * @return y coorinate output
     */
    inline float line_interp(const float x_lhs, const float y_lhs,
                             const float x_rhs, const float y_rhs,
                             const float x_out) const noexcept;
signals:
    /**
     * @brief updata_progress_bar Signal update progress bar
     */
    void updata_progress_bar(const int);

    /**
     * @brief end_of_job signal end of job
     */
    void end_of_job(const std::shared_ptr<QImage>);
};

#endif // LRT_THREAD_HPP
