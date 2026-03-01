#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class SidebarMenu : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarMenu(QWidget* parent = nullptr);
    ~SidebarMenu();

signals:
    void libraryClicked();
    void favoritesClicked();
    void settingsClicked();
    void aboutClicked();

private:
    void setupUI();
    QPushButton* createMenuButton(const QString& icon, const QString& text);

    QVBoxLayout* mainLayout;
    QLabel* headerLabel;
    QPushButton* libraryButton;
    QPushButton* favoritesButton;
    QPushButton* settingsButton;
    QPushButton* aboutButton;
};