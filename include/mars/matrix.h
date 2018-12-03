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
public:
    typedef T ValueType;
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

    /**
     * 转置矩阵计算。
     */
    TMatrix<T> transpose() const {
        TMatrix<T> mr(this->cols(), this->rows());
        for(size_t i = 0; i < mr.rows(); ++i ) {
            for( size_t j = 0; j < mr.cols(); ++j) {
                mr(i, j) = (*this)(j, i);
            }
        }
        return std::move(mr);
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

/**
 * 矩阵除法，m中每个元素除以value.
 */
template <class T>
TMatrix<T> operator / (const TMatrix<T> &m, T value) {
    TMatrix<T> mr(m.rows(), m.cols());
    for(size_t i = 0; i < mr.rows(); ++i) {
        for ( size_t j = 0; j < mr.cols(); ++j) {
            mr(i, j) = m(i, j) / value; 
        }
    }
    return std::move(mr);
}

template<class T>
struct Matop { 

    // 转置矩阵计算
    static TMatrix<T> transpose(const TMatrix<T> &m);

    // 矩阵行交换。在原矩阵上执行r1行与r2行的位置交换，返回原矩阵的引用。
    static TMatrix<T> & row_switch(TMatrix<T> &m, size_t r1, size_t r2);

    // 矩阵行乘以指定值。在原矩阵上执行r1行乘以value值，返回原矩阵引用。
    static TMatrix<T> & row_multiply(TMatrix<T> &m, size_t r1, T value);

    // 矩阵行除以指定值。在原矩阵上执行r1行除以value值，返回原矩阵引用。
    static TMatrix<T> & row_divide(TMatrix<T> &m, size_t r1, T value);
    
    // 行r2的元素乘以value后加到行r1, 行r1值变更，r2不变。(r1) = m(r2) * value + r1
    static TMatrix<T> & row_multiply_add(TMatrix<T> &m, size_t r1, size_t r2, T value);

    // 转换为规范阶梯矩阵
    static TMatrix<T> & reduced_row_echelon(TMatrix<T> &m);
}; // end class Matop

// 矩阵转置
template<class T>
TMatrix<T> Matop<T>::transpose(const TMatrix<T> &m) {
    TMatrix<T> mr(m.cols(), m.rows());
    for(size_t i = 0; i < mr.rows(); ++i ) {
        for( size_t j = 0; j < mr.cols(); ++j) {
            mr(i, j) = m(j, i);
        }
    }
    return std::move(mr);
}

// 矩阵行交换
template<class T>
TMatrix<T> & Matop<T>::row_switch(TMatrix<T> &m, size_t r1, size_t r2) {
    T temp;
    for ( size_t c = 0; c < m.cols(); ++c) {
        temp = m(r1, c);
        m(r1, c) = m(r2, c);
        m(r2, c) = temp;
    }
    return m;
}

// 矩阵行乘以指定值。在原矩阵上执行r1行乘以value值，返回原矩阵引用。
template<class T>
TMatrix<T> & Matop<T>::row_multiply(TMatrix<T> &m, size_t r1, T value) {
    for ( size_t c = 0; c < m.cols(); ++c) {
        T & r = m( r1, c );
        r *= value;
    }
    return m;
}

// 矩阵行除以指定值。在原矩阵上执行r1行除以value值，返回原矩阵引用。
template<class T>
TMatrix<T> & Matop<T>::row_divide(TMatrix<T> &m, size_t r1, T value) {
    for ( size_t c = 0; c < m.cols(); ++c) {
        T & r = m( r1, c );
        r /= value;
    }
    return m;
}

// 行r2的元素乘以value后加到行r1, 行r1值变更，r2不变。(r1) = m(r2) * value + r1
template<class T>
TMatrix<T> & Matop<T>::row_multiply_add(TMatrix<T> &m, size_t r1, size_t r2, T value) {
    for (size_t c = 0; c < m.cols(); ++c) {
        T & r = m(r1, c);
        r = m(r2, c) * value + r;
    }
    return m;
}

// 转换为规范阶梯矩阵
template<class T>
TMatrix<T> & Matop<T>::reduced_row_echelon(TMatrix<T> &m) {
    size_t rows = m.rows();
    size_t cols = m.cols();

    assert(rows <= cols);    // rows > cols是无解的
    
    // 逐行处理
    for( size_t r = 0; r < rows; ++r) {
        const T & v0 = m(r, r);
        if ( v0 == 0 ) {   // 对角线元素为0，则需要从后面找一个非零的行，进行交换，以便形成阶梯
            for ( size_t r1 = r + 1; r1 < rows; ++r1) {
                if ( m(r1, r1) != 0 ) {
                    Matop<T>::row_switch(m, r, r1);
                    break;
                }
            }
            if ( m(r, r) == 0) continue;  // 没有不等于零的，就下一行
        }

        // 当前行对角线值（即首个非零值）变为1
        Matop<T>::row_divide(m, r, m(r, r) );

        // 遍历后续的行，进行消元操作
        for( size_t n = r + 1; n < rows; ++n) {
            Matop<T>::row_multiply_add(m, n, r, m(n, r) * (-1));
        }
    } // end for r
    return m;
}

} // end namespace mars