/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

#ifndef MATRIX_H
#define MATRIX_H

#include "StaticMatrix.h"

template <typename T> class Matrix
{
public:
    /* Constructors & destructor */
    inline Matrix() : _p(NULL) {}
    inline Matrix(const Matrix<T> &other);
    inline Matrix(int m, int n, T *_data);
    Matrix(int m, int n);
    Matrix(int m, int n, T value);
    inline ~Matrix();
    /* Trivial operations */
    inline bool isNull() const;
    inline const T &operator()(const quint16 &i, const quint16 &j) const;
    inline T &operator()(const quint16 &i, const quint16 &j);
    inline const T* constData() const { return _p ? _p->d->constData() : NULL; }
    inline T* data() { detach(); return _p ? _p->d->data() : NULL; }
    void fill(T value = 0);
    inline void fillZero();
    inline int countRows() const { return _p ? _p->d->countRows() : 0; }
    inline int countCols() const { return _p ? _p->d->countCols() : 0; }
    void addIdentity();
    /* Mathematical operators */
    Matrix<T> &operator=(const Matrix<T> &other);
    bool operator==(const Matrix<T> &other) const;
private:
    void deref();
    void detach();
private:
    struct Data
    {
        StaticMatrix<T> *d;
        int n;
        inline Data(StaticMatrix<T> *d) : d(d), n(1) {}
    };
    Data *_p;
};

template <typename T> inline Matrix<T>::Matrix(const Matrix<T> &other)
{
    _p = other._p;
    if (_p)
        ++_p->n;
}

template <typename T> inline Matrix<T>::Matrix(int m, int n, T *data)
{
    if (data)
    {
        _p = new Data(new StaticMatrix<T>(m, n, data));
    } else {
        _p = NULL;
    }
}

template <typename T> Matrix<T>::Matrix(int m, int n)
{
    if ((m > 0) && (n > 0))
    {
        _p = new Data(new StaticMatrix<T>(m, n));
    } else {
        _p = NULL;
    }
}

template <typename T> Matrix<T>::Matrix(int m, int n, T value)
{
    if ((m > 0) && (n > 0))
    {
        _p = new Data(new StaticMatrix<T>(m, n, value));
    } else {
        _p = NULL;
    }
}

template <typename T> inline Matrix<T>::~Matrix()
{
    deref();
}

template <typename T> inline bool Matrix<T>::isNull() const
{
    return !_p;
}

template <typename T> inline const T &Matrix<T>::operator()(const quint16 &i, const quint16 &j) const
{
    ASSERT(_p);
    return (*_p->d)(i, j);
}

template <typename T> inline T &Matrix<T>::operator()(const quint16 &i, const quint16 &j)
{
    detach();
    ASSERT(_p);
    return (*_p->d)(i, j);
}

template <typename T> void Matrix<T>::fill(T value)
{
    detach();
    ASSERT(_p);
    _p->d->fill(value);
}

template <typename T> inline void Matrix<T>::fillZero()
{
    detach();
    ASSERT(_p);
    _p->d->fillZero();
}

template <typename T> void Matrix<T>::addIdentity()
{
    detach();
    ASSERT(_p);
    _p->d->addIdentity();
}

template <typename T> Matrix<T> &Matrix<T>::operator=(const Matrix<T> &other)
{
    deref();
    _p = other._p;
    if (_p)
        ++_p->n;
    return *this;
}

template <typename T> bool Matrix<T>::operator==(const Matrix<T> &other) const
{
    if (_p && other._p)
    {
        if (_p == other._p)
            return true;
        return (*_p->d) == (*other._p->d);
    }
    return !(_p || other._p);
}

template <typename T> bool Matrix<T>::operator!=(const Matrix<T> &other) const
{
    if (_p && other._p)
    {
        if (_p == other._p)
            return false;
        return !((*_p->d) == (*other._p->d));
    }
    return _p || other._p;
}

template <typename T> void Matrix<T>::deref()
{
    if (_p)
    {
        if (--_p->n <= 0)
        {
            delete _p->d;
            delete _p;
        }
        _p = NULL;
    }
}

template <typename T> void Matrix<T>::detach()
{
    if (_p && (_p->n > 1))
    {
        --_p->n;
        _p = new Data(new StaticMatrix<T>(*_p->d));
    }
}

#endif // MATRIX_H
