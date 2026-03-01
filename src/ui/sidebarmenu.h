#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QColor>

class SidebarMenu : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarMenu(QWidget* parent = nullptr);
    ~SidebarMenu();

    void applyMenuColor(const QColor& color);

signals:
    void libraryClicked();
    void favoritesClicked();
    void settingsClicked();
    void termsClicked();     // NOVO: página de termos
    void aboutClicked();

private:
    void setupUI();
    QPushButton* createMenuButton(const QString& icon, const QString& text);

    QVBoxLayout* mainLayout;
    QLabel*      headerLabel;
    QPushButton* libraryButton;
    QPushButton* favoritesButton;
    QPushButton* settingsButton;
    QPushButton* termsButton;    // NOVO
    QPushButton* aboutButton;
};
