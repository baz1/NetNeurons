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
private:
    struct Data
    {
        StaticMatrix<T> *d;
        int n;
        inline Data(StaticMatrix<T> *d) : d(d), n(1) {}
    };
    Data *_p;
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
    Matrix<T> &addIdentity();
    /* Mathematical operators */
    Matrix<T> &operator=(const Matrix<T> &other);
    bool operator==(const Matrix<T> &other) const;
    bool operator!=(const Matrix<T> &other) const;
    Matrix<T> &operator+=(const Matrix<T> &other);
    Matrix<T> &operator-=(const Matrix<T> &other);
    inline Matrix<T> operator+() const { return *this; }
    Matrix<T> operator-() const;
    Matrix<T> operator+(const Matrix<T> &other) const;
    Matrix<T> operator-(const Matrix<T> &other) const;
    Matrix<T> &operator*=(const T &c);
    inline Matrix<T> &operator/=(const T &c) { return ((*this) *= (1 / c)); }
    Matrix<T> &operator*=(const Matrix<T> &other);
    inline Matrix<T> &partialProduct(const Matrix<T> &m1, const Matrix<T> &m2, int i1, int i2, int j1, int j2);
    inline Matrix<T> transpose() const;
    inline Matrix<T> timesTranspose(const Matrix<T> &other) const;
    inline T det() const;
    Matrix<T> &operator/=(const Matrix<T> &other);
    /* Cut and merge operations */
    static Matrix<T> mergeH(const Matrix<T> &m1, const Matrix<T> &m2);
    static Matrix<T> mergeV(const Matrix<T> &m1, const Matrix<T> &m2);
    inline Matrix<T> &cut(const Matrix<T> &other, int di = 0, int dj = 0, int si = 0, int sj = 0, int sm = INT_MAX, int sn = INT_MAX);
    /* Other functions */
    void print(FILE *stream, const char *(*toString) (T), const char *prepend = "  ") const;
public:
    static Matrix<T> prepareProduct(const Matrix<T> &m1, const Matrix<T> &m2);
private:
    inline Matrix(StaticMatrix<T> *d) : _p(new Data(d)) {}
    void deref();
    void detach();
};

template <typename T> inline Matrix<T>::Matrix(const Matrix<T> &other)
{
    _p = other._p;
    if (_p)
    {
        ASSERT_INT(((unsigned long long) _p->n) + 1ULL);
        ++_p->n;
    }
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
    ASSERT(_p);
    if (_p->n > 1)
    {
        --_p->n;
        _p = new Data(new StaticMatrix<T>(_p->d->countRows(), _p->d->countCols(), value));
    } else {
        _p->d->fill(value);
    }
}

template <typename T> inline void Matrix<T>::fillZero()
{
    ASSERT(_p);
    if (_p->n > 1)
    {
        --_p->n;
        _p = new Data(new StaticMatrix<T>(_p->d->countRows(), _p->d->countCols()));
    } else {
        _p->d->fillZero();
    }
}

template <typename T> Matrix<T> &Matrix<T>::addIdentity()
{
    detach();
    ASSERT(_p);
    _p->d->addIdentity();
    return *this;
}

template <typename T> Matrix<T> &Matrix<T>::operator=(const Matrix<T> &other)
{
    deref();
    _p = other._p;
    if (_p)
    {
        ASSERT_INT(((unsigned long long) _p->n) + 1ULL);
        ++_p->n;
    }
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

template <typename T> Matrix<T> &Matrix<T>::operator+=(const Matrix<T> &other)
{
    ASSERT(_p && other._p);
    detach();
    (*_p->d) += (*other._p->d); // No pb if the same pointer.
    return *this;
}

template <typename T> Matrix<T> &Matrix<T>::operator-=(const Matrix<T> &other)
{
    ASSERT(_p && other._p);
    detach();
    (*_p->d) -= (*other._p->d); // No pb if the same pointer.
    return *this;
}

template <typename T> Matrix<T> Matrix<T>::operator-() const
{
    ASSERT(_p);
    return Matrix<T>(_p->d->getOpposite());
}

template <typename T> Matrix<T> Matrix<T>::operator+(const Matrix<T> &other) const
{
    ASSERT(_p && other._p);
    Matrix<T> result(new StaticMatrix<T>(*_p->d));
    (*result._p->d) += (*other._p->d); // No pb if the same pointer.
    return result;
}

template <typename T> Matrix<T> Matrix<T>::operator-(const Matrix<T> &other) const
{
    ASSERT(_p && other._p);
    Matrix<T> result(new StaticMatrix<T>(*_p->d));
    (*result._p->d) -= (*other._p->d); // No pb if the same pointer.
    return result;
}

template <typename T> Matrix<T> &Matrix<T>::operator*=(const T &c)
{
    ASSERT(_p);
    detach();
    (*_p->d) *= c;
    return *this;
}

template <typename T> Matrix<T> &Matrix<T>::operator*=(const Matrix<T> &other)
{
    ASSERT(_p && other._p);
    StaticMatrix<T> *tmp = _p->d->getProduct(*other._p->d); // No pb if the same pointer.
    if (--_p->n <= 0)
    {
        delete _p->d;
        delete _p;
    }
    _p = new Data(tmp);
    return *this;
}

template <typename T> inline Matrix<T> &Matrix<T>::partialProduct(const Matrix<T> &m1, const Matrix<T> &m2, int i1, int i2, int j1, int j2)
{
    ASSERT(_p && m1._p && m2._p);
    detach();
    ASSERT((_p != m1._p) && (_p != m2._p)); // Avoid doing nasty stuff
    _p->d->partialProduct(*m1._p->d, *m2._p->d, i1, i2, j1, j2); // No pb if m1 and m2 hold the same pointer.
    return *this;
}

template <typename T> inline Matrix<T> Matrix<T>::transpose() const
{
    ASSERT(_p);
    return Matrix<T>(_p->d->getTranspose());
}

template <typename T> inline Matrix<T> Matrix<T>::timesTranspose(const Matrix<T> &other) const
{
    ASSERT(_p && other._p);
    return Matrix<T>(_p->d->timesTranspose(*other._p->d)); // No pb if the same pointer.
}

template <typename T> inline T Matrix<T>::det() const
{
    ASSERT(_p);
    return _p->d->det();
}

template <typename T> Matrix<T> &Matrix<T>::operator/=(const Matrix<T> &other)
{
    ASSERT(_p && other._p);
    if (_p == other._p)
    {
        /* Speedup in case of trivial division */
        StaticMatrix<T> *m = new StaticMatrix<T>(_p->d->countRows(), _p->d->countCols(), (T) 0);
        if (--_p->n <= 0)
        {
            delete _p->d;
            delete _p;
        }
        m->addIdentity();
        _p = new Data(m);
    } else {
        detach();
        (*_p->d) /= (*other._p->d); // No pb if the same pointer.
    }
    return *this;
}

template <typename T> Matrix<T> Matrix<T>::mergeH(const Matrix<T> &m1, const Matrix<T> &m2)
{
    ASSERT(m1._p && m2._p);
    return Matrix<T>(StaticMatrix<T>::mergeH(*m1._p->d, *m2._p->d)); // No pb if the same pointer.
}

template <typename T> Matrix<T> Matrix<T>::mergeV(const Matrix<T> &m1, const Matrix<T> &m2)
{
    ASSERT(m1._p && m2._p);
    return Matrix<T>(StaticMatrix<T>::mergeV(*m1._p->d, *m2._p->d)); // No pb if the same pointer.
}

template <typename T> inline Matrix<T> &Matrix<T>::cut(const Matrix<T> &other, int di, int dj, int si, int sj, int sm, int sn)
{
    ASSERT(_p && other._p);
    detach();
    _p->d->cut(*other._p->d, di, dj, si, sj, sm, sn); // Expected behavior if the same pointer.
    return *this;
}

template <typename T> void Matrix<T>::print(FILE *stream, const char *(*toString) (T), const char *prepend) const
{
    if (_p)
    {
        _p->d->print(stream, toString, prepend);
    } else {
        fprintf(stream, "%s[NULL]\n", prepend);
    }
}

template <typename T> Matrix<T> Matrix<T>::prepareProduct(const Matrix<T> &m1, const Matrix<T> &m2)
{
    ASSERT(m1._p && m2._p);
    return Matrix<T>(StaticMatrix<T>::prepareProduct(*m1._p->d, *m2._p->d)); // No pb if the same pointer.
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
