#include <QtCore>

#include "src/Matrix.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    // TODO (please, return here later!)
    Matrix<double> m(2, 2), f;
    m.addIdentity();
    f = m;
    return 0;
}
