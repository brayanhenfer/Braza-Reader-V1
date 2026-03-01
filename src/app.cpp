#include "app.h"
#include "ui/mainwindow.h"
#include <QGuiApplication>
#include <QScreen>

BrazaReader::BrazaReader()
    : mainWindow(std::make_unique<MainWindow>())
{
}

BrazaReader::~BrazaReader() = default;

void BrazaReader::run()
{
    mainWindow->setWindowTitle("BrazaReader");
    mainWindow->setGeometry(0, 0, 800, 480);
    mainWindow->show();
}
