#include "sidebarmenu.h"
#include <QFont>
#include <QPixmap>

SidebarMenu::SidebarMenu(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

SidebarMenu::~SidebarMenu() = default;

void SidebarMenu::setupUI()
{
    setFixedWidth(250);
    setStyleSheet("background-color: #1e6432;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Logo: tenta carregar PNG, cai em texto se não encontrar ──────────────
    // Coloque logo.png em resources/ e adicione ao resources.qrc como ":/logo.png"
    logoLabel = new QLabel(this);
    logoLabel->setFixedHeight(80);
    logoLabel->setAlignment(Qt::AlignCenter);
    logoLabel->setStyleSheet(
        "background-color: rgba(0,0,0,0.2); padding: 8px;");

    QPixmap logoPix(":/logo.png");
    if (!logoPix.isNull()) {
        logoLabel->setPixmap(
            logoPix.scaled(180, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // fallback: texto
        logoLabel->setText("BrazaReader");
        logoLabel->setStyleSheet(
            "color: white; font-size: 20px; font-weight: bold; "
            "background-color: rgba(0,0,0,0.2); padding: 10px;");
    }
    mainLayout->addWidget(logoLabel);

    // ── Botões de navegação ───────────────────────────────────────────────────
    libraryButton    = createMenuButton("📚", "Biblioteca");
    favoritesButton  = createMenuButton("★",  "Favoritos");
    collectionsButton= createMenuButton("📁", "Coleções");
    settingsButton   = createMenuButton("⚙",  "Configurações");
    termsButton      = createMenuButton("📄", "Termos de Uso");
    aboutButton      = createMenuButton("ℹ",  "Sobre");

    connect(libraryButton,     &QPushButton::clicked, this, &SidebarMenu::libraryClicked);
    connect(favoritesButton,   &QPushButton::clicked, this, &SidebarMenu::favoritesClicked);
    connect(collectionsButton, &QPushButton::clicked, this, &SidebarMenu::collectionsClicked);
    connect(settingsButton,    &QPushButton::clicked, this, &SidebarMenu::settingsClicked);
    connect(termsButton,       &QPushButton::clicked, this, &SidebarMenu::termsClicked);
    connect(aboutButton,       &QPushButton::clicked, this, &SidebarMenu::aboutClicked);

    mainLayout->addWidget(libraryButton);
    mainLayout->addWidget(favoritesButton);
    mainLayout->addWidget(collectionsButton);
    mainLayout->addWidget(settingsButton);
    mainLayout->addWidget(termsButton);
    mainLayout->addWidget(aboutButton);
    mainLayout->addStretch();

    QLabel* footer = new QLabel("v1.0.0", this);
    footer->setAlignment(Qt::AlignCenter);
    footer->setFixedHeight(40);
    footer->setStyleSheet("color: rgba(255,255,255,0.5); font-size: 11px;");
    mainLayout->addWidget(footer);
}

QPushButton* SidebarMenu::createMenuButton(const QString& icon, const QString& text)
{
    QPushButton* btn = new QPushButton(QString("%1   %2").arg(icon, text), this);
    btn->setFixedHeight(55);
    btn->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 15px; "
        "text-align: left; padding-left: 20px; border: none; "
        "border-bottom: 1px solid rgba(255,255,255,0.1); }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.15); }"
        "QPushButton:hover   { background-color: rgba(255,255,255,0.08); }");
    return btn;
}

void SidebarMenu::applyMenuColor(const QColor& color)
{
    setStyleSheet(QString("background-color: %1;").arg(color.name()));
}
