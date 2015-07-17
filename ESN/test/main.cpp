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
    Matrix<double> m1(2, 4), m2(4, 2), m3;
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
    m3 = Matrix<double>(4, 4);
    DISP(m3)
    m3.cut(m1);
    DISP(m3)
    m3.cut(m2, 2, 0, 0, 0, 2, 2);
    m3.cut(m2, 2, 2, 2, 0);
    DISP(m3)
    return 0;
}
