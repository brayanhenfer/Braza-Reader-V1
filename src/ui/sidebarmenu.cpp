#include "sidebarmenu.h"
#include <QFont>
#include <QPixmap>
#include <QHBoxLayout>

SidebarMenu::SidebarMenu(QWidget* parent) : QWidget(parent) { setupUI(); }
SidebarMenu::~SidebarMenu() = default;

void SidebarMenu::setupUI()
{
    setFixedWidth(230);
    setStyleSheet("background-color:#1e6432;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Cabeçalho com logo ────────────────────────────────────────────────
    headerLabel = new QLabel(this);
    headerLabel->setFixedHeight(80);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet(
        "background-color:rgba(0,0,0,0.25);padding:8px;");

    QPixmap logoPixmap(":/logo.png");
    if (!logoPixmap.isNull()) {
        headerLabel->setPixmap(
            logoPixmap.scaled(150, 56, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        headerLabel->setText("BrazaReader");
        headerLabel->setStyleSheet(
            "color:white;font-size:20px;font-weight:bold;"
            "background-color:rgba(0,0,0,0.25);padding:8px;");
    }
    mainLayout->addWidget(headerLabel);

    // ── Separador ────────────────────────────────────────────────────────
    QWidget* sep = new QWidget(this);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background:rgba(255,255,255,0.15);");
    mainLayout->addWidget(sep);

    // ── Itens do menu ─────────────────────────────────────────────────────
    // Biblioteca — 3 livros (📚)
    libraryButton = createMenuButton("📚", "Biblioteca");
    connect(libraryButton, &QPushButton::clicked, this, &SidebarMenu::libraryClicked);
    mainLayout->addWidget(libraryButton);

    // Favoritos — estrela (★)
    favoritesButton = createMenuButton("★", "Favoritos");
    connect(favoritesButton, &QPushButton::clicked, this, &SidebarMenu::favoritesClicked);
    mainLayout->addWidget(favoritesButton);

    // Coleções — 1 livro (📖)
    collectionsButton = createMenuButton("📖", "Coleções");
    connect(collectionsButton, &QPushButton::clicked, this, &SidebarMenu::collectionsClicked);
    mainLayout->addWidget(collectionsButton);

    // Configurações — engrenagem (⚙)
    settingsButton = createMenuButton("⚙", "Configurações");
    connect(settingsButton, &QPushButton::clicked, this, &SidebarMenu::settingsClicked);
    mainLayout->addWidget(settingsButton);

    // Termos de Uso — folha (📄)
    termsButton = createMenuButton("📄", "Termos de Uso");
    connect(termsButton, &QPushButton::clicked, this, &SidebarMenu::termsClicked);
    mainLayout->addWidget(termsButton);

    // Sobre — ℹ
    aboutButton = createMenuButton("ℹ", "Sobre");
    connect(aboutButton, &QPushButton::clicked, this, &SidebarMenu::aboutClicked);
    mainLayout->addWidget(aboutButton);

    mainLayout->addStretch();

    // ── Rodapé ────────────────────────────────────────────────────────────
    QWidget* footer = new QWidget(this);
    footer->setFixedHeight(40);
    footer->setStyleSheet("background:rgba(0,0,0,0.2);");
    QHBoxLayout* fl = new QHBoxLayout(footer);
    fl->setContentsMargins(16, 0, 16, 0);
    QLabel* vLabel = new QLabel("v1.0.0", footer);
    vLabel->setStyleSheet("color:rgba(255,255,255,0.45);font-size:11px;");
    fl->addWidget(vLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addWidget(footer);
}

QPushButton* SidebarMenu::createMenuButton(const QString& icon, const QString& text)
{
    // Ícone + espaço + texto
    QPushButton* btn = new QPushButton(this);
    btn->setFixedHeight(52);
    btn->setText(QString("  %1   %2").arg(icon, text));
    btn->setStyleSheet(
        "QPushButton {"
        "  background:transparent;"
        "  color:white;"
        "  font-size:15px;"
        "  text-align:left;"
        "  padding-left:12px;"
        "  border:none;"
        "  border-bottom:1px solid rgba(255,255,255,0.08);"
        "}"
        "QPushButton:pressed { background:rgba(255,255,255,0.18); }"
        "QPushButton:hover   { background:rgba(255,255,255,0.09); }"
    );
    return btn;
}
