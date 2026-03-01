#pragma once

#include <QObject>
#include <memory>

class MainWindow;

class BrazaReader : public QObject
{
    Q_OBJECT

public:
    BrazaReader();
    ~BrazaReader();

    void run();

private:
    std::unique_ptr<MainWindow> mainWindow;
};
