#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>

class CollectionScreen : public QWidget
{
    Q_OBJECT
public:
    explicit CollectionScreen(QWidget* parent = nullptr);
    ~CollectionScreen();

signals:
    void menuClicked();

private:
    void setupUI();
};
