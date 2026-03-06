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
    void collectionsClicked();
    void settingsClicked();
    void termsClicked();
    void aboutClicked();

private:
    void setupUI();

    // Cria botão com ícone desenhado via QPixmap (não usa emoji)
    QPushButton* makeButton(const QPixmap& icon, const QString& text);

    // Ícones desenhados com QPainter — garantem renderização em qualquer distro
    static QPixmap iconBooks(int n);        // n livros verticais
    static QPixmap iconGear();              // engrenagem simples
    static QPixmap iconPage();              // folha com linhas
    static QPixmap iconStar();              // estrela
    static QPixmap iconInfo();              // letra "i" em círculo

    QVBoxLayout*  mainLayout;
    QLabel*       headerLabel;
};
