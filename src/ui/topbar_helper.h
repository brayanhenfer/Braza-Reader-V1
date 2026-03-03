#pragma once
// Helper: cria topbar padrão com logo centralizada (carrega de :/logo.png)
// Adicione ao CMakeLists.txt: target_sources(... resources.qrc)
//                            set_property(TARGET ... APPEND PROPERTY AUTORCC_OPTIONS ...)
// E ao resources.qrc: <file alias="logo.png">resources/logo.png</file>

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QColor>

namespace TopBarHelper {

inline QWidget* create(QWidget* parent,
                        QPushButton* leftBtn,
                        QPushButton* rightBtn = nullptr,
                        const QColor& color   = QColor("#1e6432"))
{
    QWidget* bar = new QWidget(parent);
    bar->setFixedHeight(50);
    bar->setStyleSheet(QString("background-color:%1;").arg(color.name()));

    QHBoxLayout* lay = new QHBoxLayout(bar);
    lay->setContentsMargins(8, 4, 8, 4);
    lay->setSpacing(6);

    if (leftBtn) {
        leftBtn->setParent(bar);
        lay->addWidget(leftBtn);
    }

    // Logo centralizada
    QLabel* logo = new QLabel(bar);
    logo->setAlignment(Qt::AlignCenter);
    QPixmap px(":/logo.png");
    if (!px.isNull()) {
        // Escala mantendo aspecto dentro de 160×38px (cabeça na topbar de 50px)
        logo->setPixmap(px.scaled(160, 38, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logo->setText("BrazaReader");
        logo->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
    }
    lay->addWidget(logo, 1);

    if (rightBtn) {
        rightBtn->setParent(bar);
        lay->addWidget(rightBtn);
    }

    return bar;
}

inline void setColor(QWidget* bar, const QColor& color)
{
    if (bar)
        bar->setStyleSheet(QString("background-color:%1;").arg(color.name()));
}

} // namespace TopBarHelper
