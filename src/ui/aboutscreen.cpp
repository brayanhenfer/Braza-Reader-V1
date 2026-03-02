#include "aboutscreen.h"
#include "topbar_helper.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>

AboutScreen::AboutScreen(QWidget* parent) : QWidget(parent) { setupUI(); }
AboutScreen::~AboutScreen() = default;

void AboutScreen::setupUI()
{
    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    menuButton = new QPushButton("☰");
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuButton, &QPushButton::clicked, this, &AboutScreen::menuClicked);

    topBar = TopBarHelper::create(this, menuButton);
    lay->addWidget(topBar);

    QWidget* content = new QWidget(this);
    content->setStyleSheet("background:#2b2b2b;");
    QVBoxLayout* cl = new QVBoxLayout(content);
    cl->setContentsMargins(30, 30, 30, 30);
    cl->setSpacing(15);

    auto makeTitle = [&](const QString& t) {
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet("color:#4CAF50;font-size:20px;font-weight:bold;");
        l->setAlignment(Qt::AlignCenter);
        return l;
    };
    auto makeBody = [&](const QString& t) {
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet("color:#ccc;font-size:13px;");
        l->setAlignment(Qt::AlignCenter);
        l->setWordWrap(true);
        return l;
    };

    QLabel* appName = new QLabel("BrazaReader", content);
    appName->setStyleSheet("color:#4CAF50;font-size:32px;font-weight:bold;");
    appName->setAlignment(Qt::AlignCenter);
    cl->addWidget(appName);
    cl->addWidget(makeBody("Versão 1.0.0"));
    cl->addSpacing(10);
    cl->addWidget(makeBody("Leitor de PDF minimalista, otimizado para\nRaspberry Pi Zero com interface touchscreen."));
    cl->addSpacing(10);
    cl->addWidget(makeTitle("Recursos"));
    cl->addWidget(makeBody(
        "Scroll contínuo · Zoom · Anotações · Marcadores\n"
        "Favoritos · Coleções · Filtro âmbar · Modo noturno\n"
        "Sépia · Cache inteligente · Progresso de leitura"));
    cl->addSpacing(10);
    cl->addWidget(makeTitle("Tecnologias"));
    cl->addWidget(makeBody("C++17  |  Qt5 Widgets  |  MuPDF  |  SQLite3"));
    cl->addStretch();
    cl->addWidget(makeBody("BrazaReader v1.0.0 — 2026"));

    lay->addWidget(content, 1);
}

void AboutScreen::setMenuColor(const QColor& color)
{
    TopBarHelper::setColor(topBar, color);
}
