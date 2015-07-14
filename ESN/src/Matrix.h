/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

#ifndef MATRIX_H
#define MATRIX_H

#include <string.h>

#ifdef QT_VERSION
  #include <QtGlobal>
  #define ASSERT(x) Q_ASSERT(x)
#else
  #define ASSERT(x) (void) 0
  #ifndef NULL
    #define NULL ((void*) 0)
  #endif
#endif

#define ASSERT_INT(x) ASSERT((x) < (1ULL << (sizeof(int) * 8 - 1)))

template <typename T> class Matrix
{
public:
    inline Matrix() : data(NULL) {}
    Matrix(const Matrix<T> &other);
    inline ~Matrix();
private:
    T *data; // data[i * n + j] for the i-th row, j-th column
    int m, n; // m rows, n columns
};

template <typename T> Matrix<T>::Matrix(const Matrix<T> &other) : m(other.m), n(other.n), data(NULL)
{
    if (other.data)
    {
        ASSERT((m > 0) && (n > 0));
        ASSERT_INT(((unsigned long long) m) * ((unsigned long long) n));
        size_t size = m * n;
        data = new T[size];
        memcpy((void*) data, (void*) other.data, size);
    }
}

template <typename T> inline Matrix<T>::~Matrix()
{
    if (data)
    {
        ASSERT((m > 0) && (n > 0));
        delete[] data;
    }
}

#endif // MATRIX_H
