#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

class SidebarMenu : public QWidget {
    Q_OBJECT
public:
    explicit SidebarMenu(QWidget* parent = nullptr);
    ~SidebarMenu();
signals:
    void libraryClicked(); void favoritesClicked(); void collectionsClicked();
    void settingsClicked(); void termsClicked();    void aboutClicked();
private:
    void setupUI();
    QPushButton* makeButton(const QPixmap& icon, const QString& text);
    static QPixmap iconBooks(int n);
    static QPixmap iconGear();
    static QPixmap iconPage();
    static QPixmap iconStar();
    static QPixmap iconInfo();
    QVBoxLayout* mainLayout;
    QLabel*      headerLabel;
};
