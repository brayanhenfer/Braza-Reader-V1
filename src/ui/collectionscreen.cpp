#include "collectionscreen.h"
#include "topbar_helper.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QPixmap>
#include <QImage>

CollectionScreen::CollectionScreen(QWidget* parent) : QWidget(parent) { setupUI(); }
CollectionScreen::~CollectionScreen() = default;

void CollectionScreen::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Topbar
    QPushButton* menuBtn = new QPushButton(QString::fromUtf8("☰"), this);
    menuBtn->setFixedSize(40, 40);
    menuBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuBtn, &QPushButton::clicked, this, &CollectionScreen::menuClicked);

    QWidget* topBar = TopBarHelper::create(this, menuBtn);
    mainLayout->addWidget(topBar);

    // Conteúdo
    QWidget* content = new QWidget(this);
    content->setStyleSheet("background:#2b2b2b;");
    QVBoxLayout* cl = new QVBoxLayout(content);
    cl->setAlignment(Qt::AlignCenter);

    QLabel* icon = new QLabel(content);
    // Desenha ícone de coleção
    QPixmap px(80, 80); px.fill(Qt::transparent);
    QPainter painter(&px);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(30, 100, 50));
    painter.drawRoundedRect(0, 10, 60, 60, 6, 6);
    painter.setBrush(QColor(40, 130, 60));
    painter.drawRoundedRect(15, 5, 60, 60, 6, 6);
    painter.setBrush(QColor(50, 160, 70));
    painter.drawRoundedRect(30, 0, 60, 65, 6, 6);
    icon->setPixmap(px);
    icon->setAlignment(Qt::AlignCenter);
    cl->addWidget(icon);

    QLabel* title = new QLabel("Coleções", content);
    title->setStyleSheet("color:white;font-size:22px;font-weight:bold;margin-top:12px;");
    title->setAlignment(Qt::AlignCenter);
    cl->addWidget(title);

    QLabel* sub = new QLabel("Organize seus livros em coleções personalizadas.\nEm breve disponível.", content);
    sub->setStyleSheet("color:#888;font-size:14px;margin-top:8px;");
    sub->setAlignment(Qt::AlignCenter);
    sub->setWordWrap(true);
    cl->addWidget(sub);

    mainLayout->addWidget(content, 1);
}
