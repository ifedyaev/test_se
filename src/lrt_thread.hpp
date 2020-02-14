#ifndef LRT_THREAD_HPP
#define LRT_THREAD_HPP
#include <QThread>
#include <memory>


class LRTThread : public QThread
{
public:
    explicit LRTThread(const std::shared_ptr<QImage>& image);
    void stop() { m_is_run = false; return; }

private:
    Q_OBJECT
    void run() override;

private:
    /* flag start-stop */
    bool m_is_run{false};

    std::shared_ptr< QImage > m_image_input{nullptr};   /* input image */
    std::shared_ptr< QImage > m_image_output{nullptr};  /* output image */

    /* radon data */
    std::vector<float> m_arr_k;
    /* axis x y */
    std::vector<float> m_arr_x;
    std::vector<float> m_arr_y;
    /* border */
    float m_ub_y{0.0f};
    float m_lb_y{0.0f};

    /**
     * @brief setup_array_k angle -> to k == tan
     */
    void setup_array_k();

    /**
     * @brief setup_array_axis_xy create Ox / Oy coodinate system
     */
    void setup_array_axis_xy();

    float inperpolation_y(const float x_out, const int32_t idx_x) const;

    void compute_left_matrix(const float cur_k, const float cur_b,
                             const int32_t ik, const int32_t iy,
                             const int32_t half_size_w,
                             std::vector<float*>& matrix);

    void compute_right_matrix(const float cur_k, const float cur_b,
                              const int32_t ik, const int32_t iy,
                              const int32_t half_size_w, const int32_t n_x,
                              std::vector<float *> &matrix);

    void set_gray_image_by_data(const std::vector<float>& buffer, const std::vector<float*>& matrix,const int32_t n_k,const int32_t n_y);

    void init_matrix(std::vector<float>& buffer,std::vector<float*>& matrix, const int32_t n_k, const int32_t n_y);

    /**
     * @brief find_imax find index maximum
     * @param data
     * @return
     */
    int32_t find_imax(const std::vector<float>& data);

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
