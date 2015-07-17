#include <QtCore>

#include "src/Matrix.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // TODO (please, return here later!)
    Matrix<double> m(2, 2), f;
    m.addIdentity();
    qDebug() << f.constData() << m.constData() << (f==m);
    f = m;
    qDebug() << f.constData() << m.constData() << (f==m);
    m.addIdentity();
    qDebug() << f.constData() << m.constData() << (f==m);
    f *= 2;
    qDebug() << f.constData() << m.constData() << (f==m);
    return 0;
}
