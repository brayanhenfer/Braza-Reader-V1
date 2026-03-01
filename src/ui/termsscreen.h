#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QColor>

class TermsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit TermsScreen(QWidget* parent = nullptr);
    ~TermsScreen();

    void setMenuColor(const QColor& color);

signals:
    void backClicked();

private:
    void setupUI();

    QWidget*     topBar;
    QPushButton* backButton;
    QLabel*      titleLabel;
    QScrollArea* scrollArea;
};
