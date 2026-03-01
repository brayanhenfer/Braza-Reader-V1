#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QColor>

class AboutScreen : public QWidget
{
    Q_OBJECT

public:
    explicit AboutScreen(QWidget* parent = nullptr);
    ~AboutScreen();

    void setMenuColor(const QColor& color);  // NOVO

signals:
    void menuClicked();

private:
    void setupUI();

    QVBoxLayout* mainLayout;
    QWidget*     topBar;
    QPushButton* menuButton;
    QLabel*      titleLabel;
};
