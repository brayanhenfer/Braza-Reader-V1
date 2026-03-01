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
    setStyleSheet("background-color: #1e6432;");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    headerLabel = new QLabel("BrazaReader", this);
    headerLabel->setFixedHeight(80);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet(
        "color: white; font-size: 20px; font-weight: bold; "
        "background-color: rgba(0,0,0,0.2); padding: 10px;"
    );
    mainLayout->addWidget(headerLabel);

    libraryButton = createMenuButton("B", "Biblioteca");
    connect(libraryButton, &QPushButton::clicked, this, &SidebarMenu::libraryClicked);
    mainLayout->addWidget(libraryButton);

    favoritesButton = createMenuButton("F", "Favoritos");
    connect(favoritesButton, &QPushButton::clicked, this, &SidebarMenu::favoritesClicked);
    mainLayout->addWidget(favoritesButton);

    settingsButton = createMenuButton("C", "Configuracoes");
    connect(settingsButton, &QPushButton::clicked, this, &SidebarMenu::settingsClicked);
    mainLayout->addWidget(settingsButton);

    aboutButton = createMenuButton("S", "Sobre");
    connect(aboutButton, &QPushButton::clicked, this, &SidebarMenu::aboutClicked);
    mainLayout->addWidget(aboutButton);

    mainLayout->addStretch();

    QLabel* footerLabel = new QLabel("v1.0.0", this);
    footerLabel->setAlignment(Qt::AlignCenter);
    footerLabel->setFixedHeight(40);
    footerLabel->setStyleSheet("color: rgba(255,255,255,0.5); font-size: 11px;");
    mainLayout->addWidget(footerLabel);
}

QPushButton* SidebarMenu::createMenuButton(const QString& icon, const QString& text)
{
    QPushButton* button = new QPushButton(QString("  %1   %2").arg(icon, text), this);
    button->setFixedHeight(55);
    button->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 16px; "
        "text-align: left; padding-left: 20px; border: none; border-bottom: 1px solid rgba(255,255,255,0.1); }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.15); }"
        "QPushButton:hover { background-color: rgba(255,255,255,0.08); }"
    );
    return button;
}