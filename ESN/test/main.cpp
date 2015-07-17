#include <QtCore>

#include "src/Matrix.h"

#define DISP(m) printf(#m ":\n"); m.print(stdout, toString);

double getEl() { return ((double) (rand() % 100)) / 10.; }

static QString str;

const char *toString(double v)
{
    str = QString::number(v);
    while (str.length() < 8)
        str.append(' ');
    return qPrintable(str);
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // TODO (please, return here later!)
    Matrix<double> m1(2, 4), m2(4, 2), p1, p2;
    srand(0);
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            m1(i, j) = getEl();
            m2(j, i) = getEl();
        }
    }
    DISP(m1) DISP(m2)
    p1 = m1;
    p1 *= m2;
    DISP(p1)
    p2 = Matrix<double>::prepareProduct(m1, m2);
    DISP(p2)
    p2.getProduct(m1, m2, 0, 0, 0, 1);
    DISP(p2)
    p2.getProduct(m1, m2, 1, 1, 0, 1);
    DISP(p2)
    return 0;
}
