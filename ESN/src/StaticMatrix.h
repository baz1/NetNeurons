/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

#ifndef STATICMATRIX_H
#define STATICMATRIX_H

#include <string.h>

#define MATRIX_MEM_CMP 0

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

template <typename T> class StaticMatrix
{
public:
    /* Constructors & destructor */
    StaticMatrix(const StaticMatrix<T> &other);
    inline StaticMatrix(int m, int n, T *_data);
    inline StaticMatrix(int m, int n);
    inline StaticMatrix(int m, int n, T value);
    inline ~StaticMatrix();
    /* Trivial operations */
    inline const T &operator()(const quint16 &i, const quint16 &j) const;
    inline T &operator()(const quint16 &i, const quint16 &j);
    inline const T* constData() const { return _data; }
    inline T* data() { return _data; }
    inline void fill(T value = 0);
    inline void fillZero();
    inline int countRows() const { return _m; }
    inline int countCols() const { return _n; }
    inline void addIdentity();
    /* Mathematical operators */
    StaticMatrix<T> &operator=(const StaticMatrix<T> &other);
    bool operator==(const StaticMatrix<T> &other) const;
    inline bool operator!=(const StaticMatrix<T> &other) const { return !((*this) == other); }
    StaticMatrix<T> &operator+=(const StaticMatrix<T> &other);
    StaticMatrix<T> &operator-=(const StaticMatrix<T> &other);
    inline StaticMatrix<T> operator+() const { return *this; }
    StaticMatrix<T> operator-() const;
public:
    StaticMatrix<T> *getOpposit() const;
private:
    T *_data; // data[i * _n + j] for the i-th row, j-th column
    int _m, _n; // _m rows, _n columns
};

template <typename T> StaticMatrix<T>::StaticMatrix(const StaticMatrix<T> &other) : _m(other._m), _n(other._n)
{
    ASSERT(other._data);
    ASSERT((_m > 0) && (_n > 0));
    ASSERT_INT(((unsigned long long) _m) * ((unsigned long long) _n));
    size_t size = _m * _n;
    _data = new T[size];
    memcpy((void*) _data, (void*) other._data, size * sizeof(T));
}

template <typename T> inline StaticMatrix<T>::StaticMatrix(int m, int n, T *data) : _m(m), _n(n), _data(data)
{
    ASSERT(data);
    ASSERT((m > 0) && (n > 0));
    ASSERT_INT(((unsigned long long) m) * ((unsigned long long) n));
}

template <typename T> inline StaticMatrix<T>::StaticMatrix(int m, int n) : _m(m), _n(n)
{
    ASSERT((m > 0) && (n > 0));
    ASSERT_INT(((unsigned long long) m) * ((unsigned long long) n));
    size_t size = m * n;
    _data = new T[size];
    memset((void*) _data, 0, size * sizeof(T));
}

template <typename T> inline StaticMatrix<T>::StaticMatrix(int m, int n, T value) : _m(m), _n(n)
{
    ASSERT((m > 0) && (n > 0));
    ASSERT_INT(((unsigned long long) m) * ((unsigned long long) n));
    size_t size = m * n;
    _data = new T[size];
    while (size)
        _data[--size] = value;
}

template <typename T> inline StaticMatrix<T>::~StaticMatrix()
{
    ASSERT(_data);
    ASSERT((_m > 0) && (_n > 0));
    delete[] _data;
    ASSERT((_data = NULL, true)); // (assignment only in debug mode)
}

template <typename T> inline const T &StaticMatrix<T>::operator()(const quint16 &i, const quint16 &j) const
{
    ASSERT(_data && (i >= 0) && (i < _m) && (j >= 0) && (j < _n));
    return _data[i * _n + j];
}

template <typename T> inline T &StaticMatrix<T>::operator()(const quint16 &i, const quint16 &j)
{
    ASSERT(_data && (i >= 0) && (i < _m) && (j >= 0) && (j < _n));
    return _data[i * _n + j];
}

template <typename T> inline void StaticMatrix<T>::fill(T value)
{
    ASSERT(_data);
    size_t size = _m * _n;
    while (size)
        _data[--size] = value;
}

template <typename T> inline void StaticMatrix<T>::fillZero()
{
    ASSERT(_data);
    memset((void*) _data, 0, _m * _n * sizeof(T));
}

template <typename T> inline void StaticMatrix<T>::addIdentity()
{
    ASSERT(_data);
    ASSERT_INT(((unsigned long long) _n) + 1);
    int min = (_m < _n) ? _m : _n, step = _n + 1;
    ASSERT_INT(((unsigned long long) min) * ((unsigned long long) step));
    min *= step;
    while (min)
        _data[min -= step] += 1;
}

template <typename T> StaticMatrix<T> &StaticMatrix<T>::operator=(const StaticMatrix<T> &other)
{
    ASSERT(_data && other._data);
    ASSERT((other._m > 0) && (other._n > 0));
    ASSERT_INT(((unsigned long long) other._m) * ((unsigned long long) other._n));
    size_t size = other._m * other._n;
    if ((_m != other._m) || (_n != other._n))
    {
        _m = other._m;
        _n = other._n;
        delete[] _data;
        _data = new T[size];
    }
    while (size)
    {
        --size;
        _data[size] = other._data[size];
    }
    return *this;
}

template <typename T> bool StaticMatrix<T>::operator==(const StaticMatrix<T> &other) const
{
    ASSERT(_data && other._data);
    if ((_m != other._m) || (_n != other._n))
        return false;
#if MATRIX_MEM_CMP
    return memcmp((const void*) _data, (const void*) other._data, ((size_t) (_n * _m)) * sizeof(T)) == 0;
#else
    size_t size = _m * _n;
    while (size)
    {
        --size;
        if (_data[size] != other._data[size])
            return false;
    }
    return true;
#endif
}

template <typename T> StaticMatrix<T> &StaticMatrix<T>::operator+=(const StaticMatrix<T> &other)
{
    ASSERT(_data && other._data);
    ASSERT((_m == other._m) || (_n == other._n));
    size_t size = _m * _n;
    while (size)
    {
        --size;
        _data[size] += other._data[size];
    }
    return *this;
}

template <typename T> StaticMatrix<T> &StaticMatrix<T>::operator-=(const StaticMatrix<T> &other)
{
    ASSERT(_data && other._data);
    ASSERT((_m == other._m) || (_n == other._n));
    size_t size = _m * _n;
    while (size)
    {
        --size;
        _data[size] -= other._data[size];
    }
    return *this;
}

template <typename T> StaticMatrix<T> StaticMatrix<T>::operator-() const
{
    ASSERT(_data);
    size_t size = _m * _n;
    T *data = new T[size];
    while (size)
    {
        --size;
        data[size] = -_data[size];
    }
    return StaticMatrix(_m, _n, data);
}

template <typename T> StaticMatrix<T> *StaticMatrix<T>::getOpposit() const
{
    ASSERT(_data);
    size_t size = _m * _n;
    T *data = new T[size];
    while (size)
    {
        --size;
        data[size] = -_data[size];
    }
    return new StaticMatrix(_m, _n, data);
}

#endif // STATICMATRIX_H
