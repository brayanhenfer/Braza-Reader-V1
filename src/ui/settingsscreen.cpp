#include "settingsscreen.h"
#include "../storage/settingsmanager.h"

#include <QHBoxLayout>
#include <QColorDialog>
#include <QFrame>
#include <QFont>
#include <QGroupBox>
#include <QScrollArea>

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

    // ── Topbar ───────────────────────────────────────────────────────────────
    topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color: #1e6432;");

    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    menuButton = new QPushButton(QString::fromUtf8("☰"), topBar);
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 24px; border: none; }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.2); border-radius: 20px; }"
    );
    connect(menuButton, &QPushButton::clicked, this, &SettingsScreen::menuClicked);

    titleLabel = new QLabel("Configurações", topBar);
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");
    topLayout->addWidget(menuButton);
    topLayout->addWidget(titleLabel, 1);
    mainLayout->addWidget(topBar);

    // ── Área de scroll para o conteúdo ───────────────────────────────────────
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #2b2b2b; }");

    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: #2b2b2b;");
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);

    // ── Grupo: Cor do menu ────────────────────────────────────────────────────
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
    // FIX: ao selecionar cor, emite settingsChanged → MainWindow aplica em toda UI
    connect(colorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(settingsManager->getMenuColor(), this, "Escolher Cor do Menu");
        if (color.isValid()) onColorSelected(color);
    });

    colorLayout->addWidget(currentColorLabel);
    colorLayout->addWidget(colorButton, 1);
    contentLayout->addWidget(colorGroup);

    // ── Grupo: Cor de fundo ───────────────────────────────────────────────────
    QGroupBox* bgGroup = new QGroupBox("Cor de Fundo", content);
    bgGroup->setStyleSheet(colorGroup->styleSheet());
    QHBoxLayout* bgLayout = new QHBoxLayout(bgGroup);

    currentBgColorLabel = new QLabel(bgGroup);
    currentBgColorLabel->setFixedSize(40, 40);
    currentBgColorLabel->setStyleSheet("background-color: #2b2b2b; border-radius: 4px; border: 2px solid #888;");

    bgColorButton = new QPushButton("Escolher Fundo", bgGroup);
    bgColorButton->setStyleSheet(colorButton->styleSheet());
    connect(bgColorButton, &QPushButton::clicked, this, [this]() {
        QColor color = QColorDialog::getColor(settingsManager->getBgColor(), this, "Escolher Cor de Fundo");
        if (color.isValid()) {
            settingsManager->setBgColor(color);
            currentBgColorLabel->setStyleSheet(
                QString("background-color: %1; border-radius: 4px; border: 2px solid #888;").arg(color.name())
            );
            emit settingsChanged();
        }
    });

    bgLayout->addWidget(currentBgColorLabel);
    bgLayout->addWidget(bgColorButton, 1);
    contentLayout->addWidget(bgGroup);

    // ── Grupo: Aparência / Modo noturno ──────────────────────────────────────
    QGroupBox* nightGroup = new QGroupBox("Aparência", content);
    nightGroup->setStyleSheet(colorGroup->styleSheet());
    QVBoxLayout* nightLayout = new QVBoxLayout(nightGroup);

    nightModeCheck = new QCheckBox("Modo Noturno", nightGroup);
    nightModeCheck->setStyleSheet("QCheckBox { color: white; font-size: 14px; spacing: 10px; }");
    // FIX: modo noturno agora emite settingsChanged → MainWindow aplica tema real
    connect(nightModeCheck, &QCheckBox::toggled, this, &SettingsScreen::onNightModeToggled);

    nightLayout->addWidget(nightModeCheck);
    contentLayout->addWidget(nightGroup);

    // ── Grupo: Presets de aparência ───────────────────────────────────────────
    QGroupBox* presetGroup = new QGroupBox("Presets de Aparência", content);
    presetGroup->setStyleSheet(colorGroup->styleSheet());
    QHBoxLayout* presetLayout = new QHBoxLayout(presetGroup);
    presetLayout->setSpacing(10);

    // Preset 1: Verde floresta (padrão)
    applyPresetButton(new QPushButton("🌿 Floresta", presetGroup),
                      "🌿 Floresta", "#1e6432", "#2b2b2b", false);
    // Preset 2: Azul oceano
    applyPresetButton(new QPushButton("🌊 Oceano", presetGroup),
                      "🌊 Oceano", "#1a4a7a", "#1e2a3a", false);
    // Preset 3: Noturno puro
    applyPresetButton(new QPushButton("🌙 Noturno", presetGroup),
                      "🌙 Noturno", "#111111", "#0d0d0d", true);
    // Preset 4: Sépia
    applyPresetButton(new QPushButton("📜 Sépia", presetGroup),
                      "📜 Sépia", "#7a5c2e", "#3a2e1e", false);

    for (int i = 0; i < presetLayout->count(); i++) {
        if (auto* btn = qobject_cast<QPushButton*>(presetLayout->itemAt(i)->widget())) {
            presetLayout->addWidget(btn);
        }
    }

    // Re-adiciona os botões corretamente
    QList<QPushButton*> presetBtns;
    struct Preset { QString label, menu, bg; bool night; };
    QList<Preset> presets = {
        {"🌿 Floresta", "#1e6432", "#2b2b2b", false},
        {"🌊 Oceano",   "#1a4a7a", "#1e2a3a", false},
        {"🌙 Noturno",  "#111111", "#0d0d0d", true },
        {"📜 Sépia",    "#7a5c2e", "#3a2e1e", false},
    };

    QHBoxLayout* presetRowLayout = new QHBoxLayout();
    presetRowLayout->setSpacing(8);
    for (const Preset& p : presets) {
        QPushButton* btn = new QPushButton(p.label, presetGroup);
        btn->setStyleSheet(
            QString("QPushButton { background-color: %1; color: white; padding: 8px; "
                    "border-radius: 6px; font-size: 12px; border: 2px solid #666; }"
                    "QPushButton:pressed { border-color: white; }").arg(p.menu)
        );
        QString menuC = p.menu, bgC = p.bg;
        bool night = p.night;
        connect(btn, &QPushButton::clicked, this, [this, menuC, bgC, night]() {
            onPresetSelected(menuC, bgC, night);
        });
        presetRowLayout->addWidget(btn);
    }

    QVBoxLayout* presetVLayout = new QVBoxLayout(presetGroup);
    presetVLayout->addLayout(presetRowLayout);
    contentLayout->addWidget(presetGroup);

    // ── Restaurar padrões ─────────────────────────────────────────────────────
    resetButton = new QPushButton("Restaurar Padrões", content);
    resetButton->setStyleSheet(
        "QPushButton { background-color: #8B0000; color: white; padding: 12px 20px; "
        "border-radius: 6px; font-size: 14px; font-weight: bold; }"
        "QPushButton:pressed { background-color: #A00000; }"
    );
    connect(resetButton, &QPushButton::clicked, this, &SettingsScreen::onResetDefaults);
    contentLayout->addWidget(resetButton);

    contentLayout->addStretch();
    scrollArea->setWidget(content);
    mainLayout->addWidget(scrollArea, 1);
}

void SettingsScreen::applyPresetButton(QPushButton*, const QString&,
                                        const QString&, const QString&, bool) {}

void SettingsScreen::loadCurrentSettings()
{
    QColor menuColor = settingsManager->getMenuColor();
    QColor bgColor   = settingsManager->getBgColor();

    currentColorLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; border: 2px solid #888;").arg(menuColor.name())
    );
    currentBgColorLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; border: 2px solid #888;").arg(bgColor.name())
    );
    nightModeCheck->setChecked(settingsManager->getNightMode());
}

void SettingsScreen::setMenuColor(const QColor& color)
{
    topBar->setStyleSheet(QString("background-color: %1;").arg(color.name()));
}

// FIX: onColorSelected salva E emite settingsChanged para MainWindow aplicar em toda UI
void SettingsScreen::onColorSelected(const QColor& color)
{
    settingsManager->setMenuColor(color);
    currentColorLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; border: 2px solid #888;").arg(color.name())
    );
    emit settingsChanged();  // ← MainWindow::onSettingsChanged → applyMenuColor em todos
}

// FIX: modo noturno emite settingsChanged
void SettingsScreen::onNightModeToggled(bool enabled)
{
    settingsManager->setNightMode(enabled);
    emit settingsChanged();
}

void SettingsScreen::onPresetSelected(const QString& menuColor, const QString& bgColor, bool nightMode)
{
    settingsManager->setMenuColor(QColor(menuColor));
    settingsManager->setBgColor(QColor(bgColor));
    settingsManager->setNightMode(nightMode);
    nightModeCheck->setChecked(nightMode);
    currentColorLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; border: 2px solid #888;").arg(menuColor)
    );
    currentBgColorLabel->setStyleSheet(
        QString("background-color: %1; border-radius: 4px; border: 2px solid #888;").arg(bgColor)
    );
    emit settingsChanged();
}

void SettingsScreen::onResetDefaults()
{
    settingsManager->resetToDefaults();
    loadCurrentSettings();
    emit settingsChanged();
}
