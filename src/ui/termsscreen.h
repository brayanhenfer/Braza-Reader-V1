#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>

class TermsScreen : public QWidget
{
    Q_OBJECT
public:
    explicit TermsScreen(QWidget* parent = nullptr);
    ~TermsScreen();

signals:
    void menuClicked();

private:
    void setupUI();
};
