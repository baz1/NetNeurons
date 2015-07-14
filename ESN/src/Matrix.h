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
    /* Constructors & destructor */
    inline Matrix() : _data(NULL) {}
    Matrix(const Matrix<T> &other);
    inline Matrix(int m, int n, T *_data);
    Matrix(int m, int n);
    Matrix(int m, int n, T value);
    inline ~Matrix();
    /* Trivial information */
    inline bool isValid() const;
    inline const T &operator()(const quint16 &i, const quint16 &j) const;
    inline T &operator()(const quint16 &i, const quint16 &j);
    inline const T* constData() const { return _data; }
    inline T* data() { return data; }
private:
    T *_data; // data[i * _n + j] for the i-th row, j-th column
    int _m, _n; // _m rows, _n columns
};

template <typename T> Matrix<T>::Matrix(const Matrix<T> &other) : _m(other._m), _n(other._n), _data(NULL)
{
    if (other._data)
    {
        ASSERT((_m > 0) && (_n > 0));
        ASSERT_INT(((unsigned long long) _m) * ((unsigned long long) _n));
        size_t size = _m * _n;
        _data = new T[size];
        memcpy((void*) _data, (void*) other._data, size * sizeof(T));
    }
}

template <typename T> inline Matrix<T>::Matrix(int m, int n, T *data) : _m(m), _n(n), _data(data)
{
    if (data)
    {
        ASSERT((m > 0) && (n > 0));
        ASSERT_INT(((unsigned long long) m) * ((unsigned long long) n));
    }
}

template <typename T> Matrix<T>::Matrix(int m, int n) : _m(m), _n(n), _data(NULL)
{
    if ((m > 0) && (n > 0))
    {
        ASSERT_INT(((unsigned long long) m) * ((unsigned long long) n));
        size_t size = m * n;
        _data = new T[size];
        memset((void*) _data, 0, size * sizeof(T));
    }
}

template <typename T> Matrix<T>::Matrix(int m, int n, T value) : _m(m), _n(n), _data(NULL)
{
    if ((m > 0) && (n > 0))
    {
        ASSERT_INT(((unsigned long long) m) * ((unsigned long long) n));
        size_t size = m * n;
        _data = new T[size];
        while (size)
            _data[--size] = value;
    }
}

template <typename T> inline Matrix<T>::~Matrix()
{
    if (_data)
    {
        ASSERT((_m > 0) && (_n > 0));
        delete[] _data;
    }
}

template <typename T> inline bool Matrix<T>::isValid() const
{
    return (bool) _data;
}

template <typename T> inline const T &Matrix<T>::operator()(const quint16 &i, const quint16 &j) const
{
    ASSERT(_data && (i >= 0) && (i < _m) && (j >= 0) && (j < _n));
    return _data[i * _n + j];
}

template <typename T> inline T &Matrix<T>::operator()(const quint16 &i, const quint16 &j)
{
    ASSERT(_data && (i >= 0) && (i < _m) && (j >= 0) && (j < _n));
    return _data[i * _n + j];
}

#endif // MATRIX_H
