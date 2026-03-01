#include <QApplication>
#include "app.h"

int main(int argc, char* argv[])
{
    QApplication qapp(argc, argv);

    BrazaReader app;
    app.run();

    return qapp.exec();
}
