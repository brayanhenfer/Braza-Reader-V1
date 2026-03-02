#pragma once
// Helper estático: cria a topbar padrão com logo PNG centralizada.
// Inclua este header em cada tela e chame TopBarHelper::create(...).

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QColor>

namespace TopBarHelper {

// Cria uma topbar padrão.
// leftWidget  = botão esquerdo (menu ou voltar), pode ser nullptr
// rightWidget = botão direito (opcional), pode ser nullptr
// Retorna o QWidget* da topbar (height=50, parentada em parent)
inline QWidget* create(QWidget* parent,
                        QPushButton* leftBtn,
                        QPushButton* rightBtn = nullptr,
                        const QColor& color   = QColor("#1e6432"))
{
    QWidget* bar = new QWidget(parent);
    bar->setFixedHeight(50);
    bar->setStyleSheet(QString("background-color: %1;").arg(color.name()));

    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(8, 4, 8, 4);
    lay->setSpacing(6);

    if (leftBtn) {
        leftBtn->setParent(bar);
        lay->addWidget(leftBtn);
    }

    // Logo centralizada — PNG ou texto fallback
    QLabel* logo = new QLabel(bar);
    logo->setAlignment(Qt::AlignCenter);
    QPixmap px(":/logo.png");
    if (!px.isNull()) {
        logo->setPixmap(px.scaled(120, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logo->setText("BrazaReader");
        logo->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    }
    lay->addWidget(logo, 1);

    if (rightBtn) {
        rightBtn->setParent(bar);
        lay->addWidget(rightBtn);
    }

    return bar;
}

// Atualiza a cor de fundo de uma topbar já criada
inline void setColor(QWidget* bar, const QColor& color)
{
    if (bar)
        bar->setStyleSheet(QString("background-color: %1;").arg(color.name()));
}

} // namespace TopBarHelper
