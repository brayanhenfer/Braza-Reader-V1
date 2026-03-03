#pragma once
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
    lay->setContentsMargins(6, 4, 6, 4);
    lay->setSpacing(0);

    // Container esquerdo fixo — garante simetria com o direito
    QWidget* leftWrap = new QWidget(bar);
    leftWrap->setFixedWidth(56);
    leftWrap->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    QHBoxLayout* ll = new QHBoxLayout(leftWrap);
    ll->setContentsMargins(0,0,0,0);
    if (leftBtn) {
        leftBtn->setParent(leftWrap);
        ll->addWidget(leftBtn, 0, Qt::AlignVCenter | Qt::AlignLeft);
    }
    lay->addWidget(leftWrap);

    // Logo — ocupa todo o espaço central com stretch
    QLabel* logo = new QLabel(bar);
    logo->setAlignment(Qt::AlignCenter);
    QPixmap px(":/logo.png");
    if (!px.isNull()) {
        logo->setPixmap(px.scaled(148, 36, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logo->setText("BrazaReader");
        logo->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
    }
    lay->addWidget(logo, 1);

    // Container direito fixo — mesmo tamanho que o esquerdo
    QWidget* rightWrap = new QWidget(bar);
    rightWrap->setFixedWidth(56);
    QHBoxLayout* rl = new QHBoxLayout(rightWrap);
    rl->setContentsMargins(0,0,0,0);
    if (rightBtn) {
        rightBtn->setParent(rightWrap);
        rl->addWidget(rightBtn, 0, Qt::AlignVCenter | Qt::AlignRight);
    }
    lay->addWidget(rightWrap);

    return bar;
}

inline void setColor(QWidget* bar, const QColor& color)
{
    if (bar) bar->setStyleSheet(QString("background-color:%1;").arg(color.name()));
}

} // namespace TopBarHelper
