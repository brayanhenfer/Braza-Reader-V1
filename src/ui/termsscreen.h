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
    void setMenuColor(const QColor& color);
signals:
    void backClicked();   // MainWindow conecta a onNavigateToLibrary
private:
    void setupUI();
    QWidget*     topBar;
    QPushButton* backButton;
};
