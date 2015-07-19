/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

#ifndef STATICMATRIX_H
#define STATICMATRIX_H

#include <string.h>
#include <stdio.h>
#include <limits.h>

#define MATRIX_MEM_CMP 0

#include <QtGlobal>
#ifdef QT_VERSION /* Are we using Qt? */
  #include <QtGlobal>
  #define ASSERT(x) Q_ASSERT(x)
  #define ABS(x) qAbs(x)
#else
  #define ASSERT(x) (void) 0
  template <typename T> inline T ABS(const T &t) { return t >= 0 ? t : -t; }
  #ifndef NULL
    #define NULL ((void*) 0)
  #endif
#endif

#define ASSERT_INT(x) ASSERT((x) <= INT_MAX)

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
    inline StaticMatrix<T> &addIdentity();
    /* Mathematical operators */
    StaticMatrix<T> &operator=(const StaticMatrix<T> &other);
    bool operator==(const StaticMatrix<T> &other) const;
    inline bool operator!=(const StaticMatrix<T> &other) const { return !((*this) == other); }
    StaticMatrix<T> &operator+=(const StaticMatrix<T> &other);
    StaticMatrix<T> &operator-=(const StaticMatrix<T> &other);
    inline StaticMatrix<T> operator+() const { return *this; }
    StaticMatrix<T> operator-() const;
    inline StaticMatrix<T> operator+(const StaticMatrix<T> &other) const;
    inline StaticMatrix<T> operator-(const StaticMatrix<T> &other) const;
    StaticMatrix<T> &operator*=(const T &c);
    inline StaticMatrix<T> &operator/=(const T &c) { return ((*this) *= (1 / c)); }
    StaticMatrix<T> &operator*=(const StaticMatrix<T> &other);
    StaticMatrix<T> &partialProduct(const StaticMatrix<T> &m1, const StaticMatrix<T> &m2, int i1, int i2, int j1, int j2);
    T det() const;
    StaticMatrix<T> &operator/=(const StaticMatrix<T> &other);
    /* Cut and merge operations */
    StaticMatrix<T> &cut(const StaticMatrix<T> &other, int di = 0, int dj = 0, int si = 0, int sj = 0, int sm = INT_MAX, int sn = INT_MAX);
    /* Other functions */
    void print(FILE *stream, const char *(*toString) (T), const char *prepend = "  ") const;
public: /* Use with caution: */
    StaticMatrix<T> *getOpposite() const;
    StaticMatrix<T> *getProduct(const StaticMatrix<T> &other) const;
    static inline StaticMatrix<T> *prepareProduct(const StaticMatrix<T> &m1, const StaticMatrix<T> &m2);
    StaticMatrix<T> *getTranspose() const;
    StaticMatrix<T> *timesTranspose(const StaticMatrix<T> &other) const;
    StaticMatrix<T> *getPseudoInverse(const T &negligible = (T) 0) const;
    /* Cut and merge operations */
    static StaticMatrix<T> *mergeH(const StaticMatrix<T> &m1, const StaticMatrix<T> &m2);
    static StaticMatrix<T> *mergeV(const StaticMatrix<T> &m1, const StaticMatrix<T> &m2);
private:
    int _m, _n; // _m rows, _n columns
    T *_data; // data[i * _n + j] for the i-th row, j-th column
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
    while (n)
        _data[--n] = value;
    n = size;
    size = _n * sizeof(T);
    while (--m)
        memcpy((void*) &_data[n -= _n], (void*) _data, size);
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
    int j = _n, i = _m;
    while (j)
        _data[--j] = value;
    j = _m * _n;
    size_t size = _n * sizeof(T);
    while (--i)
        memcpy((void*) &_data[j -= _n], (void*) _data, size);
}

template <typename T> inline void StaticMatrix<T>::fillZero()
{
    ASSERT(_data);
    memset((void*) _data, 0, _m * _n * sizeof(T));
}

template <typename T> inline StaticMatrix<T> &StaticMatrix<T>::addIdentity()
{
    ASSERT(_data);
    ASSERT_INT(((unsigned long long) _n) + 1);
    int min = (_m < _n) ? _m : _n, step = _n + 1;
    ASSERT_INT(((unsigned long long) min) * ((unsigned long long) step));
    min *= step;
    while (min)
        _data[min -= step] += 1;
    return *this;
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

template <typename T> inline StaticMatrix<T> StaticMatrix<T>::operator+(const StaticMatrix<T> &other) const
{
    ASSERT(_data && other._data);
    StaticMatrix<T> result(*this);
    result += other;
    return result;
}

template <typename T> inline StaticMatrix<T> StaticMatrix<T>::operator-(const StaticMatrix<T> &other) const
{
    ASSERT(_data && other._data);
    StaticMatrix<T> result(*this);
    result -= other;
    return result;
}

template <typename T> StaticMatrix<T> &StaticMatrix<T>::operator*=(const T &c)
{
    ASSERT(_data);
    size_t size = _m * _n;
    while (size)
        _data[--size] *= c;
    return *this;
}

template <typename T> StaticMatrix<T> &StaticMatrix<T>::operator*=(const StaticMatrix<T> &other)
{
    ASSERT(_data && other._data && (_n == other._m));
    ASSERT_INT(((unsigned long long) _m) * ((unsigned long long) other._n));
    /* We assume that the naive algorithm is sufficient with the matrices that we use here. */
    ASSERT_INT((((unsigned long long) _n) + 1ULL) * ((unsigned long long) other._n));
    int i = _m, j, k, index1, index2, index3;
    T *data = new T[(index3 = _m * other._n)];
    while (i)
    {
        --i;
        index1 = i * _n;
        j = other._n;
        while (j)
        {
            --j;
            --index3;
            data[index3] = 0;
            index2 = _n * other._n + j;
            k = _n;
            while (k)
            {
                --k;
                index2 -= other._n;
                data[index3] += _data[index1 + k] * other._data[index2];
            }
        }
    }
    delete[] _data;
    _n = other._n;
    _data = data;
    return *this;
}

template <typename T> StaticMatrix<T> &StaticMatrix<T>::partialProduct(const StaticMatrix<T> &m1, const StaticMatrix<T> &m2, int i1, int i2, int j1, int j2)
{
    ASSERT(_data && m1._data && m2._data);
    ASSERT((_m == m1._m) && (m1._n == m2._m) && (m2._n == _n));
    ASSERT((i1 >= 0) && (i1 <= i2) && (i2 < _m) && (j1 >= 0) && (j1 <= j2) && (j2 < _n));
    int i, j, k, index1, index2, index3;
    for (i = i1; i <= i2; ++i)
    {
        index1 = i * m1._n;
        index3 = i * _n + j1;
        for (j = j1; j <= j2; ++j)
        {
            _data[index3] = 0;
            index2 = m1._n * _n + j;
            k = m1._n;
            while (k)
            {
                --k;
                index2 -= _n;
                _data[index3] += m1._data[index1 + k] * m2._data[index2];
            }
            ++index3;
        }
    }
    return *this;
}

template <typename T> T StaticMatrix<T>::det() const
{
    ASSERT(_data && (_m == _n));
    /* As with the multiplication, we will settle for the UL decomposition in O(n^3) */
    size_t size = _m * _n, swapSize;
    T *copy = new T[size];
    T *tmp = new T[_n], maxAbs, tmpT;
    memcpy((void*) copy, (void*) _data, size * sizeof(T));
    bool neg = false;
    int k = _n, lindex = size - 1, index, best, step = _n + 1;
    while (--k)
    {
        maxAbs = ABS(copy[(index = lindex)]);
        best = index;
        while ((index -= _n) > 0)
        {
            T tmpAbs = ABS(copy[index]);
            if (tmpAbs > maxAbs)
            {
                maxAbs = tmpAbs;
                best = index;
            }
        }
        if (maxAbs == 0)
            return (T) 0;
        if (best != lindex)
        {
            swapSize = (k + 1) * sizeof(T);
            memcpy((void*) tmp, (void*) &copy[lindex - k], swapSize);
            memcpy((void*) &copy[lindex - k], (void*) &copy[best - k], swapSize);
            memcpy((void*) &copy[best - k], (void*) tmp, swapSize);
            neg = !neg;
        }
        index = lindex;
        maxAbs = copy[index];
        while ((index -= _n) > 0)
        {
            tmpT = copy[index];
            if (tmpT != 0)
            {
                for (best = k; best > 0; --best)
                    copy[index - best] -= tmpT * copy[lindex - best] / maxAbs;
            }
        }
        lindex -= step;
    }
    delete[] tmp;
    tmpT = copy[0];
    for (index = size - 1; index > 0; index -= step)
        tmpT *= copy[index];
    delete[] copy;
    return (neg ? -tmpT : tmpT);
}

template <typename T> StaticMatrix<T> &StaticMatrix<T>::operator/=(const StaticMatrix<T> &other)
{
    ASSERT(_data && other._data);
    ASSERT((_m == _n) && (_m == other._m) && (other._m == other._n));
    size_t size = _m * _n, swapSize1, swapSize2 = _n * sizeof(T);
    T *copy = new T[size];
    T *tmp = new T[_n], maxAbs, tmpT;
    memcpy((void*) copy, (void*) other._data, size * sizeof(T));
    int k = _n, lindex = size - 1, index, best, step = _n + 1, i1, i2;
    int lastLine = size - _n;
    while (k)
    {
        --k;
        maxAbs = ABS(copy[(index = lindex)]);
        best = index;
        while ((index -= _n) > 0)
        {
            T tmpAbs = ABS(copy[index]);
            if (tmpAbs > maxAbs)
            {
                maxAbs = tmpAbs;
                best = index;
            }
        }
        ASSERT(maxAbs != 0);
        i1 = lindex - k;
        if (best != lindex)
        {
            swapSize1 = (k + 1) * sizeof(T);
            i2 = best - k;
            memcpy((void*) tmp, (void*) &copy[i1], swapSize1);
            memcpy((void*) &copy[i1], (void*) &copy[i2], swapSize1);
            memcpy((void*) &copy[i2], (void*) tmp, swapSize1);
            memcpy((void*) tmp, (void*) &_data[i1], swapSize2);
            memcpy((void*) &_data[i1], (void*) &_data[i2], swapSize2);
            memcpy((void*) &_data[i2], (void*) tmp, swapSize2);
        }
        maxAbs = copy[lindex];
        if (maxAbs != 1)
        {
            index = _n;
            while (index > k)
            {
                --index;
                _data[i1 + index] /= maxAbs;
            }
            while (index)
            {
                --index;
                _data[i1 + index] /= maxAbs;
                copy[i1 + index] /= maxAbs;
            }
        }
        for (index = lastLine + k; index > 0; index -= _n)
        {
            if (index != lindex)
            {
                tmpT = copy[index];
                if (tmpT != 0)
                {
                    for (i2 = k; i2 > 0; --i2)
                    {
                        copy[index - i2] -= tmpT * copy[lindex - i2];
                        _data[index - i2] -= tmpT * _data[lindex - i2];
                    }
                    i2 = _n - k;
                    while (i2)
                    {
                        --i2;
                        _data[index + i2] -= tmpT * _data[lindex + i2];
                    }
                }
            }
        }
        lindex -= step;
    }
    delete[] tmp;
    delete[] copy;
    return *this;
}

template <typename T> StaticMatrix<T> &StaticMatrix<T>::cut(const StaticMatrix<T> &other, int di, int dj, int si, int sj, int sm, int sn)
{
    int tmp;
    ASSERT(_data && other._data);
    ASSERT((si >= 0) && (sj >= 0));
    if (di < 0)
    {
        si += di;
        sm += di;
        di = 0;
    }
    if (dj < 0)
    {
        sj += dj;
        sn += dj;
        dj = 0;
    }
    tmp = other._n - sj;
    if (tmp < sn)
        sn = tmp;
    tmp = _n - dj;
    if (tmp < sn)
        sn = tmp;
    if (sn <= 0)
        return *this;
    tmp = other._m - si;
    if (tmp < sm)
        sm = tmp;
    tmp = _m - di;
    if (tmp < sm)
        sm = tmp;
    dj += di * _n;
    sj += si * other._n;
    while (sm > 0)
    {
        memcpy((void*) &_data[dj], (void*) &other._data[sj], sn * sizeof(T));
        sj += other._n;
        dj += _n;
        --sm;
    }
    return *this;
}

template <typename T> void StaticMatrix<T>::print(FILE *stream, const char *(*toString) (T), const char *prepend) const
{
    ASSERT(_data);
    int index = 0;
    for (int i = 0; i < _m; ++i)
    {
        fprintf(stream, "%s[", prepend);
        for (int j = 0; j < _n; ++j)
        {
            if (j)
            {
                fprintf(stream, "  %s", toString(_data[index++]));
            } else {
                fprintf(stream, "%s", toString(_data[index++]));
            }
        }
        fprintf(stream, "]\n");
    }
}

template <typename T> StaticMatrix<T> *StaticMatrix<T>::getOpposite() const
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

template <typename T> StaticMatrix<T> *StaticMatrix<T>::getProduct(const StaticMatrix<T> &other) const
{
    ASSERT(_data && other._data && (_n == other._m));
    ASSERT_INT(((unsigned long long) _m) * ((unsigned long long) other._n));
    /* We assume that the naive algorithm is sufficient with the matrices that we use here. */
    ASSERT_INT((((unsigned long long) _n) + 1ULL) * ((unsigned long long) other._n));
    int i = _m, j, k, index1, index2, index3;
    T *data = new T[(index3 = _m * other._n)];
    while (i)
    {
        --i;
        index1 = i * _n;
        j = other._n;
        while (j)
        {
            --j;
            --index3;
            data[index3] = 0;
            index2 = _n * other._n + j;
            k = _n;
            while (k)
            {
                --k;
                index2 -= other._n;
                data[index3] += _data[index1 + k] * other._data[index2];
            }
        }
    }
    return new StaticMatrix<T>(_m, other._n, data);
}

template <typename T> inline StaticMatrix<T> *StaticMatrix<T>::prepareProduct(const StaticMatrix<T> &m1, const StaticMatrix<T> &m2)
{
    ASSERT(m1._data && m2._data && (m1._n == m2._m));
    ASSERT_INT(((unsigned long long) m1._m) * ((unsigned long long) m2._n));
    ASSERT_INT((((unsigned long long) m1._n) + 1ULL) * ((unsigned long long) m2._n)); // For the following calculus
    return new StaticMatrix<T>(m1._m, m2._n, new T[m1._m * m2._n]);
}

template <typename T> StaticMatrix<T> *StaticMatrix<T>::getTranspose() const
{
    ASSERT(_data);
    int size = _m * _n;
    ASSERT_INT(((unsigned long long) size) + ((unsigned long long) _n));
    T *data = new T[size];
    int i2 = size, i1s = _n, i1, i;
    while (i2)
    {
        i1 = size + (--i1s);
        i = _m;
        while (i)
        {
            --i;
            i1 -= _n;
            data[--i2] = _data[i1];
        }
    }
    return new StaticMatrix<T>(_n, _m, data);
}

template <typename T> StaticMatrix<T> *StaticMatrix<T>::timesTranspose(const StaticMatrix<T> &other) const
{
    ASSERT(_data && other._data);
    ASSERT(_n == other._n);
    int index, i1, si2, i2, k;
    T *data = new T[(index = _m * other._m)];
    i1 = _m * _n;
    si2 = other._m * _n;
    while (i1)
    {
        i1 -= _n;
        i2 = si2;
        while (i2)
        {
            i2 -= _n;
            data[--index] = 0;
            k = _n;
            while (k)
            {
                --k;
                data[index] += _data[i1 + k] * other._data[i2 + k];
            }
        }
    }
    return new StaticMatrix<T>(_m, other._m, data);
}

template <typename T> StaticMatrix<T> *StaticMatrix<T>::getPseudoInverse(const T &negligible) const
{
    ASSERT(_data);
    bool trans;
    const StaticMatrix<T> *A, *tA;
    StaticMatrix<T> *AtA, *tiAtA, *result;
    if (_n < _m)
    {
        tA = this;
        A = tA->getTranspose();
        trans = true;
    } else {
        A = this;
        trans = false;
    }
    AtA = A->timesTranspose(*A);
    if (trans)
        delete A;
    tiAtA = new StaticMatrix<T>(AtA->_n, AtA->_n, (T) 0);
    tiAtA->addIdentity();
    T *tmp = new T[AtA->_n], maxAbs, tmpT;
    size_t swapSize1, swapSize2 = AtA->_n * sizeof(T);
    int k = AtA->_n, lindex = AtA->_n * AtA->_n, index, best, step = AtA->_n + 1, i1, i2;
    int lastLine = lindex - AtA->_n, kmin = 0, nextCol = 0;
    --lindex;
    while (k > kmin)
    {
        --k;
        maxAbs = ABS(AtA->_data[(index = lindex)]);
        best = index;
        while ((index -= AtA->_n) > 0)
        {
            T tmpAbs = ABS(AtA->_data[index]);
            if (tmpAbs > maxAbs)
            {
                maxAbs = tmpAbs;
                best = index;
            }
        }
        if (maxAbs <= negligible)
        {
            i1 = lindex - k;
            memcpy((void*) tmp, (void*) &tiAtA->_data[i1], swapSize2);
            memcpy((void*) &tiAtA->_data[i1], (void*) &tiAtA->_data[nextCol], swapSize2);
            memcpy((void*) &tiAtA->_data[nextCol], (void*) tmp, swapSize2);
            nextCol += AtA->_n;
            index = lastLine + k;
            i2 = lastLine + kmin;
            while (index >= 0)
            {
                tmpT = AtA->_data[index];
                AtA->_data[index] = AtA->_data[i2];
                AtA->_data[i2] = tmpT;
                index -= AtA->_n;
                i2 -= AtA->_n;
            }
            ++kmin;
            ++k;
            continue;
        }
        maxAbs = AtA->_data[best];
        if (best != lindex)
        {
            i1 = lindex - k;
            swapSize1 = (k + 1) * sizeof(T);
            i2 = best - k;
            memcpy((void*) tmp, (void*) &AtA->_data[i1], swapSize1);
            memcpy((void*) &AtA->_data[i1], (void*) &AtA->_data[i2], swapSize1);
            memcpy((void*) &AtA->_data[i2], (void*) tmp, swapSize1);
            index = lastLine + k;
            i2 = lastLine + (best / AtA->_n);
            if (maxAbs != 1)
            {
                while (index >= lindex)
                {
                    tmpT = tiAtA->_data[index];
                    tiAtA->_data[index] = tiAtA->_data[i2] / maxAbs;
                    tiAtA->_data[i2] = tmpT;
                    index -= AtA->_n;
                    i2 -= AtA->_n;
                }
                i1 = lindex;
                while (index >= 0)
                {
                    tmpT = tiAtA->_data[index];
                    tiAtA->_data[index] = tiAtA->_data[i2] / maxAbs;
                    tiAtA->_data[i2] = tmpT;
                    AtA->_data[--i1] /= maxAbs;
                    index -= AtA->_n;
                    i2 -= AtA->_n;
                }
            } else {
                while (index >= 0)
                {
                    tmpT = tiAtA->_data[index];
                    tiAtA->_data[index] = tiAtA->_data[i2];
                    tiAtA->_data[i2] = tmpT;
                    index -= AtA->_n;
                    i2 -= AtA->_n;
                }
            }
        } else {
            if (maxAbs != 1)
            {
                index = lastLine + k;
                while (index >= lindex)
                {
                    tiAtA->_data[index] /= maxAbs;
                    index -= AtA->_n;
                }
                i1 = lindex;
                while (index >= 0)
                {
                    tiAtA->_data[index] /= maxAbs;
                    AtA->_data[--i1] /= maxAbs;
                    index -= AtA->_n;
                }
            }
        }
        i1 = lindex + AtA->_n - 1 - k;
        for (index = lastLine + k; index > 0; index -= AtA->_n, --i1)
        {
            if (index != lindex)
            {
                tmpT = AtA->_data[index];
                if (tmpT != 0)
                {
                    best = 0;
                    for (i2 = k; i2 > 0; --i2)
                    {
                        AtA->_data[index - i2] -= tmpT * AtA->_data[lindex - i2];
                        best += AtA->_n;
                        tiAtA->_data[i1 - best] -= tmpT * tiAtA->_data[lindex - best];
                    }
                    best = (AtA->_n - k) * AtA->_n;
                    while (best)
                    {
                        best -= AtA->_n;
                        tiAtA->_data[i1 + best] -= tmpT * tiAtA->_data[lindex + best];
                    }
                }
            }
        }
        lindex -= step;
    }
    if (kmin)
    {
        i2 = kmin * AtA->_n;
        while (kmin)
        {
            --kmin;
            index = lastLine + kmin;
            do {
                --i2;
                AtA->_data[i2] = tiAtA->_data[index];
                tiAtA->_data[index] = (T) 0;
                index -= AtA->_n;
            } while (index >= 0);
            i1 = i2 + k;
            i2 += AtA->_n;
            best = AtA->_n - kmin;
            nextCol = best * AtA->_n + i1;
            while (i2 > i1)
            {
                --i2;
                --best;
                nextCol -= AtA->_n;
                tmpT = AtA->_data[i2];
                if (tmpT != 0)
                {
                    index = lastLine + kmin;
                    do {
                        tiAtA->_data[index] -= tmpT * tiAtA->_data[index + best];
                        index -= AtA->_n;
                    } while (index >= 0);
                    index = k;
                    do {
                        AtA->_data[i1 - index] -= tmpT * AtA->_data[nextCol - index];
                        --index;
                    } while (index);
                }
            }
            i2 -= k;
        }
        while (k)
        {
            --k;
            maxAbs = ABS(AtA->_data[(index = lindex)]);
            best = index;
            while ((index -= AtA->_n) > 0)
            {
                T tmpAbs = ABS(AtA->_data[index]);
                if (tmpAbs > maxAbs)
                {
                    maxAbs = tmpAbs;
                    best = index;
                }
            }
            ASSERT(maxAbs != 0);
            maxAbs = AtA->_data[best];
            if (best != lindex)
            {
                i1 = lindex - k;
                swapSize1 = (k + 1) * sizeof(T);
                i2 = best - k;
                memcpy((void*) tmp, (void*) &AtA->_data[i1], swapSize1);
                memcpy((void*) &AtA->_data[i1], (void*) &AtA->_data[i2], swapSize1);
                memcpy((void*) &AtA->_data[i2], (void*) tmp, swapSize1);
                index = lastLine + k;
                i2 = lastLine + (best / AtA->_n);
                if (maxAbs != 1)
                {
                    while (index >= lindex)
                    {
                        tmpT = tiAtA->_data[index];
                        tiAtA->_data[index] = tiAtA->_data[i2] / maxAbs;
                        tiAtA->_data[i2] = tmpT;
                        index -= AtA->_n;
                        i2 -= AtA->_n;
                    }
                    i1 = lindex;
                    while (index >= 0)
                    {
                        tmpT = tiAtA->_data[index];
                        tiAtA->_data[index] = tiAtA->_data[i2] / maxAbs;
                        tiAtA->_data[i2] = tmpT;
                        AtA->_data[--i1] /= maxAbs;
                        index -= AtA->_n;
                        i2 -= AtA->_n;
                    }
                } else {
                    while (index >= 0)
                    {
                        tmpT = tiAtA->_data[index];
                        tiAtA->_data[index] = tiAtA->_data[i2];
                        tiAtA->_data[i2] = tmpT;
                        index -= AtA->_n;
                        i2 -= AtA->_n;
                    }
                }
            } else {
                if (maxAbs != 1)
                {
                    index = lastLine + k;
                    while (index >= lindex)
                    {
                        tiAtA->_data[index] /= maxAbs;
                        index -= AtA->_n;
                    }
                    i1 = lindex;
                    while (index >= 0)
                    {
                        tiAtA->_data[index] /= maxAbs;
                        AtA->_data[--i1] /= maxAbs;
                        index -= AtA->_n;
                    }
                }
            }
            i1 = lindex + AtA->_n - 1 - k;
            for (index = lastLine + k; index > 0; index -= AtA->_n, --i1)
            {
                if (index != lindex)
                {
                    tmpT = AtA->_data[index];
                    if (tmpT != 0)
                    {
                        best = 0;
                        for (i2 = k; i2 > 0; --i2)
                        {
                            AtA->_data[index - i2] -= tmpT * AtA->_data[lindex - i2];
                            best += AtA->_n;
                            tiAtA->_data[i1 - best] -= tmpT * tiAtA->_data[lindex - best];
                        }
                        best = (AtA->_n - k) * AtA->_n;
                        while (best)
                        {
                            best -= AtA->_n;
                            tiAtA->_data[i1 + best] -= tmpT * tiAtA->_data[lindex + best];
                        }
                    }
                }
            }
            lindex -= step;
        }
    }
    delete tmp;
    delete AtA;
    if (trans)
    {
        result = tiAtA->timesTranspose(*tA);
    } else {
        tA = A->getTranspose();
        result = tA->timesTranspose(*tiAtA);
        delete tA;
    }
    return result;
}

template <typename T> StaticMatrix<T> *StaticMatrix<T>::mergeH(const StaticMatrix<T> &m1, const StaticMatrix<T> &m2)
{
    ASSERT(m1._data && m2._data && (m1._m == m2._m));
    ASSERT_INT(((unsigned long long) m1._m) * (((unsigned long long) m1._n) + ((unsigned long long) m2._n)));
    int m3_n = m1._n + m2._n;
    int i = m1._m, i1 = m1._m * m1._n, i2 = m2._m * m2._n, i3 = m1._m * m3_n;
    T *data = new T[i3];
    while (i)
    {
        --i;
        i2 -= m2._n;
        i3 -= m2._n;
        memcpy((void*) &data[i3], (void*) &m2._data[i2], m2._n * sizeof(T));
        i1 -= m1._n;
        i3 -= m1._n;
        memcpy((void*) &data[i3], (void*) &m1._data[i1], m1._n * sizeof(T));
    }
    return new StaticMatrix<T>(m1._m, m3_n, data);
}

template <typename T> StaticMatrix<T> *StaticMatrix<T>::mergeV(const StaticMatrix<T> &m1, const StaticMatrix<T> &m2)
{
    ASSERT(m1._data && m2._data && (m1._n == m2._n));
    ASSERT_INT((((unsigned long long) m1._m) + ((unsigned long long) m2._m)) * ((unsigned long long) m1._n));
    int m3_m = m1._m + m2._m;
    T *data = new T[m3_m * m1._n];
    size_t m1_size = m1._m * m1._n;
    memcpy((void*) data, (void*) m1._data, m1_size * sizeof(T));
    memcpy((void*) &data[m1_size], (void*) m2._data, m2._m * m2._n * sizeof(T));
    return new StaticMatrix<T>(m3_m, m1._n, data);
}

#endif // STATICMATRIX_H
