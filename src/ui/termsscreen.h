#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QColor>

class TermsScreen : public QWidget
{
    Q_OBJECT
public:
    explicit TermsScreen(QWidget* parent = nullptr);
    ~TermsScreen();

    void applyMenuColor(const QColor& c);
    void applyBgColor(const QColor& c);

signals:
    void menuClicked();

private:
    void setupUI();

    QWidget* topBar = nullptr;
};
