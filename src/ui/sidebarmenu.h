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
    void collectionsClicked();   // NOVO
    void settingsClicked();
    void termsClicked();         // NOVO
    void aboutClicked();

private:
    void setupUI();
    QPushButton* createMenuButton(const QString& icon, const QString& text);

    QVBoxLayout* mainLayout;
    QLabel*      logoLabel;      // logo PNG ou texto
    QPushButton* libraryButton;
    QPushButton* favoritesButton;
    QPushButton* collectionsButton; // NOVO
    QPushButton* settingsButton;
    QPushButton* termsButton;
    QPushButton* aboutButton;
};
