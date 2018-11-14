#pragma once

#include <stdint.h>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <sstream>

namespace mars 
{
template <class T>
class TMatrix  {
private:
    size_t m_rows;
    size_t m_cols;
    T    * m_array;

public:
    TMatrix() : m_rows(0), m_cols(0), m_array(nullptr) {}

    TMatrix(T *array, size_t rows, size_t cols) : m_rows(rows), m_cols(cols), m_array(nullptr) {
        size_t len = rows * cols;
        if ( len > 0 ) { 
            m_array = new T[len];
            for ( size_t i = 0; i < rows; ++i)
                for( size_t j = 0; j < cols; ++j) {
                    size_t offset = i * cols + j;
                    m_array[offset] = array[offset];
                }
                    
        }
    }

    TMatrix(size_t rows, size_t cols) : m_rows(rows), m_cols(cols), m_array(nullptr)  {
        size_t len = rows * cols;
        if ( len > 0 ) m_array = new T[len];
    }
    TMatrix(size_t rows, size_t cols, T initval) : TMatrix(rows, cols) {
        size_t len = rows * cols;
        for ( size_t i = 0; i < len; ++i ) m_array[i] = initval;
    }

    TMatrix(const TMatrix & m) : TMatrix(m.m_rows, m.m_cols) {
        size_t len = m_rows * m_cols;
        if ( m_array ) memcpy(m_array, m.m_array, len * sizeof(T));
    }

    TMatrix(TMatrix && m) : m_rows(m.m_rows), m_cols(m.m_cols), m_array(m.m_array) {
        m.m_rows = m.m_cols = 0;
        m.m_array = nullptr;
    }

    ~TMatrix() {
        if ( m_array ) delete[] m_array;
        m_rows = m_cols = 0;
        m_array = nullptr;
    }

    TMatrix & operator=(const TMatrix & m) {
        if ( this == &m ) return *this;
        m_rows = m.m_rows;
        m_cols = m.m_cols;
        if ( m_array ) {
            delete[] m_array;
            m_array = nullptr;
        }
        if ( m.m_array ) {
            size_t len = m_rows * m_cols;
            m_array = new T[len];
            memcpy(m_array, m.m_array, len * sizeof(T));
        }
        return *this;
    }

    TMatrix & operator=(TMatrix && m) {
        if ( this == &m ) return *this;
        m_rows = m.m_rows;
        m_cols = m.m_cols;
        m_array = m.m_array;

        m.m_rows = m.m_cols = 0;
        m.m_array = nullptr;
        return *this;
    }

    const T & operator()(size_t row, size_t col) const { 
        if ( row >= m_rows ) {
            std::ostringstream oss;
            oss<<"matrix() bad row: "<<row<<", rows: "<<m_rows;
            throw std::out_of_range(oss.str().c_str());
        }
        if ( col >= m_cols ) {
            std::ostringstream oss;
            oss<<"matrix() bad col: "<<col<<", cols: "<<m_cols;
            throw std::out_of_range(oss.str().c_str());
        }

        return m_array[row * m_cols + col];
    }

    T & operator()(size_t row, size_t col) {
        if ( row >= m_rows ) {
            std::ostringstream oss;
            oss<<"matrix() bad row: "<<row<<", rows: "<<(m_rows);
            throw std::out_of_range(oss.str().c_str());
        }
        if ( col >= m_cols ) {
            std::ostringstream oss;
            oss<<"matrix() bad col: "<<col<<", cols: "<<(m_cols);
            throw std::out_of_range(oss.str().c_str());
        }

        return m_array[row * m_cols + col];
    }

    size_t rows() const { return m_rows; }
    size_t cols() const { return m_cols; }

    T    get(size_t row, size_t col) const { return this->operator()(row, col); }

    void set(size_t row, size_t col, T value) { 
        this->operator()(row, col) = value;
    }

    template<class OutputStream>
    void print(OutputStream &oss) {
        oss<<std::endl;
        for (size_t i = 0; i < m_rows; ++i) {
            for( size_t j = 0; j < m_cols; ++j) {
                oss<<"\t"<<this->get(i, j);
            }
            oss<<std::endl;
        }
    }
}; // end class TMatrix

typedef TMatrix<double> DoubleMatrix;
typedef TMatrix<int> IntMatrix;
typedef DoubleMatrix Matrix;

/**
 * 矩阵加法运算，计算m1 + m2. m1和m2具有相同的rows和cols。
 */
template<class T>
TMatrix<T> operator+(const TMatrix<T> &m1, const TMatrix<T> &m2) {
    assert(m1.rows() == m2.rows() && m1.cols() == m2.cols());
    TMatrix<T> mr(m1.rows(), m1.cols());
    for( size_t i = 0; i < m1.rows(); ++i) {
        for( size_t j = 0; j < m1.cols(); ++j) {
            mr(i, j) = m1(i, j) + m2(i, j);
        }
    }
    return std::move(mr);
}

/**
 * 矩阵加法运算，计算m + value。m中的每个元素都加上value值
 */
template<class T>
TMatrix<T> operator+(const TMatrix<T> &m, T value) {
    TMatrix<T> mr(m.rows(), m.cols());
    for( size_t i = 0; i < m.rows(); ++i) {
        for( size_t j = 0; j < m.cols(); ++j) {
            mr(i, j) = m(i, j) + value;
        }
    }
    return std::move(mr);
}

/**
 * 矩阵减法运算，计算m1 - m2，m1和m2具有相同的rows和cols。
 */
template<class T>
TMatrix<T> operator-(const TMatrix<T> &m1, const TMatrix<T> &m2) {
    assert(m1.rows() == m2.rows() && m1.cols() == m2.cols());
    TMatrix<T> mr(m1.rows(), m1.cols());
    for( size_t i = 0; i < m1.rows(); ++i) {
        for( size_t j = 0; j < m1.cols(); ++j) {
            mr(i, j) = m1(i, j) - m2(i, j);
        }
    }
    return std::move(mr);
}

/**
 * 矩阵加法运算，计算m - value。m中的每个元素都减去value值
 */
template<class T>
TMatrix<T> operator-(const TMatrix<T> &m, T value) {
    TMatrix<T> mr(m.rows(), m.cols());
    for( size_t i = 0; i < m.rows(); ++i) {
        for( size_t j = 0; j < m.cols(); ++j) {
            mr(i, j) = m(i, j) - value;
        }
    }
    return std::move(mr);
}

/**
 * 矩阵乘法，计算m1 * m2，m1.cols == m2.rows, 
 */ 
template <class T>
TMatrix<T> operator*(const TMatrix<T> &m1, const TMatrix<T> &m2) {
    assert(m1.cols() == m2.rows());
    TMatrix<T> mr(m1.rows(), m2.cols());
    for(size_t i = 0; i < mr.rows(); ++i) {
        for ( size_t j = 0; j < mr.cols(); ++j) {
            T value(0);
            for(size_t t = 0; t < m1.cols(); ++t ) {
                value += ( m1(i, t) * m2(t, j) );
            }
            mr(i, j) = value;
        }
    }
    return std::move(mr);
}

/**
 * 矩阵乘法，计算m * value，m中的每个元素乘以value, 
 */ 
template <class T>
TMatrix<T> operator*(const TMatrix<T> &m, T value) {
    TMatrix<T> mr(m.rows(), m.cols());
    for(size_t i = 0; i < mr.rows(); ++i) {
        for ( size_t j = 0; j < mr.cols(); ++j) {
            mr(i, j) = m(i, j) * value; 
        }
    }
    return std::move(mr);
}


} // end namespace mars