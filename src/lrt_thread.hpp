#ifndef LRT_THREAD_HPP
#define LRT_THREAD_HPP
#include <QThread>


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

    std::shared_ptr< QImage > m_image_input;

    std::shared_ptr< QImage > m_image_output;
    /* radon data */
    std::vector<float> m_arr_k;
    /* axis x y */
    std::vector<float> m_arr_x;
    std::vector<float> m_arr_y;
    /* border */
    float m_ub_y;
    float m_lb_y;

    void setup_array_k();

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

    int32_t find_imax(const std::vector<float>& data);

    /**
     * @brief line_interp line interp
     * @param x_lhs
     * @param y_lhs
     * @param x_rhs
     * @param y_rhs
     * @param x_out
     * @return
     */
    inline float line_interp(const float x_lhs, const float y_lhs,
                             const float x_rhs, const float y_rhs,
                             const float x_out) const noexcept;

signals:

public slots:
};

#endif // LRT_THREAD_HPP
