#include <QtCore>

#include "src/Matrix.h"

#define DISP(m) printf(#m ":\n"); m.print(stdout, toString);

double getEl() { return ((double) (rand() % 200 - 100)) / 100.; }

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
    Matrix<double> m1(3, 3), m2(3, 3), m3(3, 3);
    srand(0);
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            m1(i, j) = getEl();
            m2(j, i) = getEl();
        }
    }
    DISP(m1) DISP(m2)
    printf("det(m1) = %lf\n", m1.det());
    m3.addIdentity();
    m3 /= m1;
    DISP(m3)
    printf("det(m1) = %lf\n", 1/m3.det());
    m3 *= m1;
    DISP(m3)
    return 0;
}
