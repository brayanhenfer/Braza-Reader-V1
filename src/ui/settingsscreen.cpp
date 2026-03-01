#include "settingsscreen.h"
#include "../storage/settingsmanager.h"

#include <QHBoxLayout>
#include <QColorDialog>
#include <QFrame>
#include <QFont>
#include <QGroupBox>

SettingsScreen::SettingsScreen(QWidget* parent)
    : QWidget(parent)
    , settingsManager(std::make_unique<SettingsManager>())
{
    setupUI();
    loadCurrentSettings();
}

SettingsScreen::~SettingsScreen() = default;

void SettingsScreen::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color: #1e6432;");

    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    menuButton = new QPushButton(QString::fromUtf8("\xe2\x98\xb0"), topBar);
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 24px; border: none; }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.2); border-radius: 20px; }"
    );
    connect(menuButton, &QPushButton::clicked, this, &SettingsScreen::menuClicked);

    titleLabel = new QLabel("Configuracoes", topBar);
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");

    topLayout->addWidget(menuButton);
    topLayout->addWidget(titleLabel, 1);
    mainLayout->addWidget(topBar);

    QWidget* content = new QWidget(this);
    content->setStyleSheet("background-color: #2b2b2b;");
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);

    // Cor do menu
    QGroupBox* colorGroup = new QGroupBox("Cor do Menu", content);
    colorGroup->setStyleSheet(
        "QGroupBox { color: white; font-size: 14px; font-weight: bold; "
        "border: 1px solid #555; border-radius: 8px; margin-top: 10px; padding-top: 15px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }"
    );
    QHBoxLayout* colorLayout = new QHBoxLayout(colorGroup);

    currentColorLabel = new QLabel(colorGroup);
    currentColorLabel->setFixedSize(40, 40);
    currentColorLabel->setStyleSheet("background-color: #1e6432; border-radius: 4px; border: 2px solid #888;");

    colorButton = new QPushButton("Escolher Cor", colorGroup);
    colorButton->setStyleSheet(
        "QPushButton { background-color: #444; color: white; padding: 10px 20px; "
        "border-radius: 6px; font-size: 14px; }"
        "QPushButton:pressed { background-color: #555; }"
    );
    connect(colorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(settingsManager->getMenuColor(), this, "Escolher Cor do Menu");
        if (color.isValid()) {
            onColorSelected(color);
        }
    });

    colorLayout->addWidget(currentColorLabel);
    colorLayout->addWidget(colorButton, 1);
    contentLayout->addWidget(colorGroup);

    // Modo noturno
    QGroupBox* nightGroup = new QGroupBox("Aparencia", content);
    nightGroup->setStyleSheet(
        "QGroupBox { color: white; font-size: 14px; font-weight: bold; "
        "border: 1px solid #555; border-radius: 8px; margin-top: 10px; padding-top: 15px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 5px; }"
    );
    QVBoxLayout* nightLayout = new QVBoxLayout(nightGroup);

    nightModeCheck = new QCheckBox("Modo Noturno", nightGroup);
    nightModeCheck->setStyleSheet("QCheckBox { color: white; font-size: 14px; spacing: 10px; }");
    connect(nightModeCheck, &QCheckBox::toggled, this, &SettingsScreen::onNightModeToggled);

    nightLayout->addWidget(nightModeCheck);
    contentLayout->addWidget(nightGroup);

    // Restaurar padroes
    resetButton = new QPushButton("Restaurar Padroes", content);
    resetButton->setStyleSheet(
        "QPushButton { background-color: #8B0000; color: white; padding: 12px 20px; "
        "border-radius: 6px; font-size: 14px; font-weight: bold; }"
        "QPushButton:pressed { background-color: #A00000; }"
    );
    connect(resetButton, &QPushButton::clicked, this, &SettingsScreen::onResetDefaults);
    contentLayout->addWidget(resetButton);

    contentLayout->addStretch();
    mainLayout->addWidget(content, 1);
}

void SettingsScreen::loadCurrentSettings()
{
    QColor menuColor = settingsManager->getMenuColor();
    currentColorLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; border: 2px solid #888;").arg(menuColor.name())
    );
    nightModeCheck->setChecked(settingsManager->getNightMode());
}

void SettingsScreen::onColorSelected(const QColor& color)
{
    settingsManager->setMenuColor(color);
    currentColorLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; border: 2px solid #888;").arg(color.name())
    );
    emit settingsChanged();
}

void SettingsScreen::onNightModeToggled(bool enabled)
{
    settingsManager->setNightMode(enabled);
    emit settingsChanged();
}

void SettingsScreen::onResetDefaults()
{
    settingsManager->resetToDefaults();
    loadCurrentSettings();
    emit settingsChanged();
}