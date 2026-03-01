#include "sidebarmenu.h"
#include <QFont>

SidebarMenu::SidebarMenu(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

SidebarMenu::~SidebarMenu() = default;

void SidebarMenu::setupUI()
{
    setFixedWidth(250);
    // Cor inicial; será sobrescrita por applyMenuColor()
    setStyleSheet("background-color: #1e6432;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Cabeçalho com logo centralizada ──────────────────────────────────────
    headerLabel = new QLabel("BrazaReader", this);
    headerLabel->setFixedHeight(80);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet(
        "color: white; font-size: 20px; font-weight: bold; "
        "background-color: rgba(0,0,0,0.2); padding: 10px;"
    );
    mainLayout->addWidget(headerLabel);

    // ── Botões sem letra prefixada ────────────────────────────────────────────
    //   FIX: antes era createMenuButton("B", "Biblioteca") que gerava "  B   Biblioteca"
    //   Agora usa emoji como ícone limpo
    libraryButton   = createMenuButton(QString::fromUtf8("📚"), "Biblioteca");
    favoritesButton = createMenuButton(QString::fromUtf8("★"),  "Favoritos");
    settingsButton  = createMenuButton(QString::fromUtf8("⚙"),  "Configurações");
    termsButton     = createMenuButton(QString::fromUtf8("📄"), "Termos de Uso");
    aboutButton     = createMenuButton(QString::fromUtf8("ℹ"),  "Sobre");

    connect(libraryButton,   &QPushButton::clicked, this, &SidebarMenu::libraryClicked);
    connect(favoritesButton, &QPushButton::clicked, this, &SidebarMenu::favoritesClicked);
    connect(settingsButton,  &QPushButton::clicked, this, &SidebarMenu::settingsClicked);
    connect(termsButton,     &QPushButton::clicked, this, &SidebarMenu::termsClicked);
    connect(aboutButton,     &QPushButton::clicked, this, &SidebarMenu::aboutClicked);

    mainLayout->addWidget(libraryButton);
    mainLayout->addWidget(favoritesButton);
    mainLayout->addWidget(settingsButton);
    mainLayout->addWidget(termsButton);
    mainLayout->addWidget(aboutButton);

    mainLayout->addStretch();

    QLabel* footerLabel = new QLabel("v1.0.0", this);
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setFixedHeight(40);
    footerLabel->setStyleSheet("color: rgba(255,255,255,0.5); font-size: 11px;");
    mainLayout->addWidget(footerLabel);
}

// FIX: removido o parâmetro "icon" que era uma letra solta (B/F/C/S).
//      Agora recebe emoji ou símbolo real como ícone.
QPushButton* SidebarMenu::createMenuButton(const QString& icon, const QString& text)
{
    // Formato limpo: "📚  Biblioteca" — sem letra aleatória no início
    QPushButton* button = new QPushButton(QString("%1   %2").arg(icon, text), this);
    button->setFixedHeight(55);
    button->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 16px; "
        "text-align: left; padding-left: 20px; border: none; "
        "border-bottom: 1px solid rgba(255,255,255,0.1); }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.15); }"
        "QPushButton:hover   { background-color: rgba(255,255,255,0.08); }"
    );
    return button;
}

// Aplica dinamicamente a cor do menu (sidebar + topbars de cada tela)
void SidebarMenu::applyMenuColor(const QColor& color)
{
    setStyleSheet(QString("background-color: %1;").arg(color.name()));
}
