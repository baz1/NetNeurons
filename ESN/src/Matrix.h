/*
 * Copyright (c) 2015, Rémi Bazin <bazin.remi@gmail.com>
 * All rights reserved.
 * See LICENSE for licensing details.
 */

#ifndef MATRIX_H
#define MATRIX_H

#ifdef QT_VERSION
#include <QtGlobal>
#else
// TODO ASSERT macro
#endif

#ifndef NULL
#define NULL ((void*) 0)
#endif

template <typename T> class Matrix
{
public:
    explicit inline Matrix() : data(NULL), m(0), n(0) {}
private:
    T **data; // First index is the row index (~ Y axis)
    int m, n; // m rows, n columns
};

#endif // MATRIX_H
