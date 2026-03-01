#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class AboutScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AboutScreen(QWidget* parent = nullptr);
    ~AboutScreen();

signals:
    void menuClicked();

private:
    void setupUI();

    QVBoxLayout* mainLayout;
    QWidget* topBar;
    QPushButton* menuButton;
    QLabel* titleLabel;
};