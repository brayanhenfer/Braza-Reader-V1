#include "aboutscreen.h"
#include "topbar_helper.h"
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFont>

AboutScreen::AboutScreen(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

AboutScreen::~AboutScreen() = default;

void AboutScreen::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Topbar com logo (TopBarHelper) ─────────────────────────────────────
    menuButton = new QPushButton(QString::fromUtf8("☰"), this);
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 24px; border: none; }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.2); border-radius: 20px; }"
    );
    connect(menuButton, &QPushButton::clicked, this, &AboutScreen::menuClicked);
    topBar = TopBarHelper::create(this, menuButton);
    mainLayout->addWidget(topBar);

    // Conteudo
    QWidget* content = new QWidget(this);
    content->setStyleSheet("background-color: #2b2b2b;");
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(15);

    // Logo / nome
    QLabel* appName = new QLabel("BrazaReader", content);
    appName->setStyleSheet("color: #4CAF50; font-size: 32px; font-weight: bold;");
    appName->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(appName);

    QLabel* version = new QLabel("Versao 1.0.0", content);
    version->setStyleSheet("color: #aaa; font-size: 16px;");
    version->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(version);

    contentLayout->addSpacing(20);

    // Descricao
    QLabel* description = new QLabel(
        "Leitor de PDF minimalista e eficiente,\n"
        "otimizado para dispositivos com\n"
        "hardware limitado.\n\n"
        "Especialmente projetado para\n"
        "Raspberry Pi Zero.",
        content
    );
    description->setStyleSheet("color: white; font-size: 14px;");
    description->setAlignment(Qt::AlignCenter);
    description->setWordWrap(true);
    contentLayout->addWidget(description);

    contentLayout->addSpacing(20);

    // Recursos
    QLabel* featuresTitle = new QLabel("Recursos", content);
    featuresTitle->setStyleSheet("color: #4CAF50; font-size: 18px; font-weight: bold;");
    featuresTitle->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(featuresTitle);

    QLabel* features = new QLabel(
        "- Leitura offline de PDFs\n"
        "- Sistema de favoritos\n"
        "- Historico de leitura\n"
        "- Temas configuraveis\n"
        "- Modo noturno\n"
        "- Ajuste de fonte\n"
        "- Interface touch otimizada\n"
        "- Cache inteligente de paginas",
        content
    );
    features->setStyleSheet("color: #ccc; font-size: 13px;");
    features->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(features);

    contentLayout->addSpacing(20);

    // Tecnologias
    QLabel* techTitle = new QLabel("Tecnologias", content);
    techTitle->setStyleSheet("color: #4CAF50; font-size: 18px; font-weight: bold;");
    techTitle->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(techTitle);

    QLabel* tech = new QLabel(
        "C++17 | Qt5 Widgets | MuPDF | SQLite3",
        content
    );
    tech->setStyleSheet("color: #ccc; font-size: 13px;");
    tech->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(tech);

    contentLayout->addStretch();

    // Rodape
    QLabel* footer = new QLabel("BrazaReader v1.0.0 - 2026", content);
    footer->setStyleSheet("color: #666; font-size: 11px;");
    footer->setAlignment(Qt::AlignCenter);
    contentLayout->addWidget(footer);

    mainLayout->addWidget(content, 1);
}