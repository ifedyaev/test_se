#ifndef MATRIX_HPP
#define MATRIX_HPP
/*******************************
 *    class Matrix
 *( Column | Row ) - Fortran Style
 *
 *******************************
 */
#include <vector>
#include <algorithm>
#include <stdint.h>

template<typename T>
class Matrix
{
public:
    Matrix(const int32_t n_col, const int32_t n_row);

    /**
     * @brief operator [] set value matrix
     * @param col- column matrix
     * @return pointer start column
     * a[col][row] = value;
     */
    T* operator[] (const int64_t col){ return m_ptr_data[col]; };

    /**
     * @brief operator [] get value
     * @param col - column matrix
     * @return pointer start column
     * value = a[col][row];
     */
    const T* operator[] (const int64_t col) const{ return m_ptr_data[col]; };

    /**
     * @brief normalization_on_gray Convert value -> to Gray color
     */
    void normalization_on_gray();

private:
    std::vector<T> m_data;
    std::vector<T*> m_ptr_data;

    /**
     * @brief find_imax find index max in array
     * @param data input array
     * @return index max value
     */
    int32_t find_imax(const std::vector<T> &data) const;
};


template< typename T>
Matrix<T>::Matrix(const int32_t n_col, const int32_t n_row)
{
    /* creat buffer data */
    m_data.resize(n_col*n_row);
    const T value = static_cast<T>(0);
    std::fill(m_data.begin(),m_data.end(),value);

    /* create move of data */
    m_ptr_data.resize(n_col);
    int32_t i;
    /* fill pointer */
    for(i = 0; i < n_col; ++i){
        m_ptr_data[i] = m_data.data() + i*n_row;
    }
}

template<typename T>
void Matrix<T>::normalization_on_gray()
{
    const int32_t idx_max = find_imax(m_data);
    T scale_matrix{1.0f};
    const T max_val = m_data[idx_max];
    if(max_val != 0.0f){
        scale_matrix = 255.0f/max_val;
    }

    const int32_t n = static_cast<int32_t>(m_data.size());
    int32_t i;
    for(i = 0; i < n; ++i){
        m_data[i] *= scale_matrix;
    }

    return;
}

template<typename T>
int32_t Matrix<T>::find_imax(const std::vector<T> &data) const
{
    static constexpr T eps = std::numeric_limits<T>::epsilon()*10;
    /* check size */
    if(data.size() == 0){ return 0;}

    int32_t idx_max;
    T val_max = std::numeric_limits<T>::lowest();
    const int32_t n_data = static_cast<int32_t>(data.size());

    int32_t i;
    for(i = 0; i < n_data; ++i){
        const T value = data[i];
        if(val_max - value < eps){
            val_max = value;
            idx_max = i;
        }
    }
    return idx_max;
}

#endif // MATRIX_HPP
