#include "settingsscreen.h"
#include "../storage/settingsmanager.h"

#include <QHBoxLayout>
#include <QColorDialog>
#include <QGroupBox>
#include <cstdlib>
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
    QVBoxLayout* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // ── Topbar ───────────────────────────────────────────────────────────────
    topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color:#1e6432;");
    QHBoxLayout* topLay = new QHBoxLayout(topBar);
    topLay->setContentsMargins(10, 5, 10, 5);

    menuButton = new QPushButton("☰", topBar);
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuButton, &QPushButton::clicked, this, &SettingsScreen::menuClicked);

    titleLabel = new QLabel("Configurações", topBar);
    titleLabel->setStyleSheet("color:white;font-size:18px;font-weight:bold;");
    topLay->addWidget(menuButton);
    topLay->addWidget(titleLabel, 1);
    mainLay->addWidget(topBar);

    // ── Scroll ───────────────────────────────────────────────────────────────
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;background:#2b2b2b;}");

    QWidget* content = new QWidget();
    content->setStyleSheet("background:#2b2b2b;");
    QVBoxLayout* cLay = new QVBoxLayout(content);
    cLay->setContentsMargins(16, 16, 16, 16);
    cLay->setSpacing(16);

    auto makeGroup = [](const QString& title, QWidget* parent) {
        QGroupBox* g = new QGroupBox(title, parent);
        g->setStyleSheet(
            "QGroupBox{color:white;font-size:14px;font-weight:bold;"
            "border:1px solid #555;border-radius:8px;margin-top:10px;padding-top:15px;}"
            "QGroupBox::title{subcontrol-origin:margin;left:15px;padding:0 5px;}");
        return g;
    };

    // ── Grupo: Cor do Menu ────────────────────────────────────────────────────
    QGroupBox* colorGroup = makeGroup("Cor do Menu", content);
    QHBoxLayout* colorLay = new QHBoxLayout(colorGroup);
    currentColorLabel = new QLabel(colorGroup);
    currentColorLabel->setFixedSize(40, 40);
    currentColorLabel->setStyleSheet("background:#1e6432;border-radius:4px;border:2px solid #888;");
    colorButton = new QPushButton("Escolher Cor", colorGroup);
    colorButton->setStyleSheet("QPushButton{background:#444;color:white;padding:10px 20px;border-radius:6px;font-size:14px;}"
                                "QPushButton:pressed{background:#555;}");
    connect(colorButton, &QPushButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(settingsManager->getMenuColor(), this, "Cor do Menu");
        if (c.isValid()) onColorSelected(c);
    });
    colorLay->addWidget(currentColorLabel);
    colorLay->addWidget(colorButton, 1);
    cLay->addWidget(colorGroup);

    // ── Grupo: Cor de Fundo ───────────────────────────────────────────────────
    QGroupBox* bgGroup = makeGroup("Cor de Fundo", content);
    QHBoxLayout* bgLay = new QHBoxLayout(bgGroup);
    currentBgLabel = new QLabel(bgGroup);
    currentBgLabel->setFixedSize(40, 40);
    currentBgLabel->setStyleSheet("background:#2b2b2b;border-radius:4px;border:2px solid #888;");
    bgColorButton = new QPushButton("Escolher Fundo", bgGroup);
    bgColorButton->setStyleSheet(colorButton->styleSheet());
    connect(bgColorButton, &QPushButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(settingsManager->getBgColor(), this, "Cor de Fundo");
        if (c.isValid()) {
            settingsManager->setBgColor(c);
            currentBgLabel->setStyleSheet(
                QString("background:%1;border-radius:4px;border:2px solid #888;").arg(c.name()));
            emit settingsChanged(); // MainWindow aplica no fundo real
        }
    });
    bgLay->addWidget(currentBgLabel);
    bgLay->addWidget(bgColorButton, 1);
    cLay->addWidget(bgGroup);

    // ── Grupo: Brilho ─────────────────────────────────────────────────────────
    QGroupBox* brightGroup = makeGroup("Brilho da Tela", content);
    QVBoxLayout* brightLay = new QVBoxLayout(brightGroup);
    QLabel* brightVal = new QLabel("100%", brightGroup);
    brightVal->setStyleSheet("color:white;font-size:13px;");
    brightnessSlider = new QSlider(Qt::Horizontal, brightGroup);
    brightnessSlider->setRange(20, 100);
    brightnessSlider->setValue(100);
    brightnessSlider->setStyleSheet(
        "QSlider::groove:horizontal{background:#444;height:6px;border-radius:3px;}"
        "QSlider::handle:horizontal{background:#4CAF50;width:22px;height:22px;"
        "margin:-8px 0;border-radius:11px;}"
        "QSlider::sub-page:horizontal{background:#1e6432;border-radius:3px;}");
    connect(brightnessSlider, &QSlider::valueChanged, this, [this, brightVal](int v) {
        brightVal->setText(QString("%1%").arg(v));
        settingsManager->setBrightness(v);
        emit settingsChanged();
    });
    brightLay->addWidget(brightnessSlider);
    brightLay->addWidget(brightVal);
    cLay->addWidget(brightGroup);

    // ── Grupo: Temperatura de Cor (âmbar) ────────────────────────────────────
    QGroupBox* amberGroup = makeGroup("Temperatura de Cor (Filtro Âmbar)", content);
    QVBoxLayout* amberLay = new QVBoxLayout(amberGroup);
    QLabel* amberVal = new QLabel("Desligado", amberGroup);
    amberVal->setStyleSheet("color:#FFA040;font-size:13px;");
    amberSlider = new QSlider(Qt::Horizontal, amberGroup);
    amberSlider->setRange(0, 100);
    amberSlider->setValue(0);
    amberSlider->setStyleSheet(
        "QSlider::groove:horizontal{background:#444;height:6px;border-radius:3px;}"
        "QSlider::handle:horizontal{background:#FF9500;width:22px;height:22px;"
        "margin:-8px 0;border-radius:11px;}"
        "QSlider::sub-page:horizontal{background:#FF6600;border-radius:3px;}");
    connect(amberSlider, &QSlider::valueChanged, this, [this, amberVal](int v) {
        amberVal->setText(v == 0 ? "Desligado" : QString("Intensidade %1%").arg(v));
        settingsManager->setAmberIntensity(v);
        emit settingsChanged();
    });
    amberLay->addWidget(amberSlider);
    amberLay->addWidget(amberVal);
    cLay->addWidget(amberGroup);

    // ── Grupo: Aparência ──────────────────────────────────────────────────────
    QGroupBox* appearGroup = makeGroup("Aparência", content);
    QVBoxLayout* appearLay = new QVBoxLayout(appearGroup);

    nightModeCheck = new QCheckBox("🌙 Modo Noturno (fundo escuro)", appearGroup);
    nightModeCheck->setStyleSheet("QCheckBox{color:white;font-size:14px;spacing:10px;}");
    connect(nightModeCheck, &QCheckBox::toggled, this, &SettingsScreen::onNightModeToggled);
    appearLay->addWidget(nightModeCheck);

    sepiaCheck = new QCheckBox("📜 Modo Sépia (sobreposição de tom)", appearGroup);
    sepiaCheck->setStyleSheet("QCheckBox{color:white;font-size:14px;spacing:10px;}");
    connect(sepiaCheck, &QCheckBox::toggled, this, [this](bool on) {
        settingsManager->setSepiaEnabled(on);
        emit settingsChanged();
    });
    appearLay->addWidget(sepiaCheck);

    cLay->addWidget(appearGroup);

    // ── Grupo: Orientação do Ecrã ─────────────────────────────────────────────
    // No RPi com X11 usamos xrandr. Em Wayland usar wlr-randr.
    QGroupBox* orientGroup = makeGroup("Orientação do Ecrã", content);
    QHBoxLayout* orientLay = new QHBoxLayout(orientGroup);
    auto makeOrientBtn = [&](const QString& label, const QString& xrandrArg) {
        QPushButton* b = new QPushButton(label, orientGroup);
        b->setStyleSheet("QPushButton{background:#444;color:white;padding:10px;border-radius:6px;font-size:12px;}"
                          "QPushButton:pressed{background:#1e6432;}");
        connect(b, &QPushButton::clicked, this, [xrandrArg]() {
            // Tenta as saídas mais comuns do RPi automaticamente
            QString cmd = QString(
                "xrandr --output HDMI-1 --rotate %1 2>/dev/null || "
                "xrandr --output DSI-1  --rotate %1 2>/dev/null || "
                "xrandr --output LVDS-1 --rotate %1 2>/dev/null"
            ).arg(xrandrArg);
            system(cmd.toStdString().c_str());
        });
        return b;
    };
    orientLay->addWidget(makeOrientBtn("↕ Retrato",   "normal"));
    orientLay->addWidget(makeOrientBtn("↔ Paisagem",  "left"));
    orientLay->addWidget(makeOrientBtn("↕ Inv.",      "inverted"));
    orientLay->addWidget(makeOrientBtn("↔ Pais.2",   "right"));
    cLay->addWidget(orientGroup);

    // ── Grupo: Presets ────────────────────────────────────────────────────────
    QGroupBox* presetGroup = makeGroup("Presets de Aparência", content);
    QVBoxLayout* presetLay = new QVBoxLayout(presetGroup);

    struct Preset {
        QString label, menu, bg;
        bool night, sepia;
        int amber;
    };
    QList<Preset> presets = {
        {"🌿 Floresta (padrão)", "#1e6432", "#2b2b2b", false, false, 0},
        {"🌊 Oceano",            "#1a4a7a", "#1e2a3a", false, false, 0},
        {"🌙 Noturno Escuro",    "#111111", "#0a0a0a", true,  false, 0},
        {"📜 Sépia Clássico",   "#7a5c2e", "#3a2e1e", false, true,  20},
        {"🕯️  Luz de Vela",      "#8B4513", "#1a0e00", true,  false, 60},
        {"🌸 Rosa Suave",        "#8B3a62", "#2a1a22", false, false, 0},
        {"🖤 Preto Total",       "#000000", "#000000", true,  false, 0},
        {"☀️  Dia Ensolarado",   "#2196F3", "#F5F5F5", false, false, 0},
    };

    // Grid 2 colunas de presets
    QHBoxLayout* presetRow = nullptr;
    for (int i = 0; i < presets.size(); i++) {
        if (i % 2 == 0) {
            presetRow = new QHBoxLayout();
            presetRow->setSpacing(8);
            presetLay->addLayout(presetRow);
        }
        const Preset& p = presets[i];
        QPushButton* btn = new QPushButton(p.label, presetGroup);
        btn->setStyleSheet(
            QString("QPushButton{background:%1;color:white;padding:10px;"
                    "border-radius:6px;font-size:12px;border:2px solid #555;}"
                    "QPushButton:pressed{border-color:white;}").arg(p.menu));
        QString mc=p.menu, bc=p.bg; bool nt=p.night, sp=p.sepia; int am=p.amber;
        connect(btn, &QPushButton::clicked, this, [this, mc, bc, nt, sp, am]() {
            settingsManager->setMenuColor(QColor(mc));
            settingsManager->setBgColor(QColor(bc));
            settingsManager->setNightMode(nt);
            settingsManager->setSepiaEnabled(sp);
            settingsManager->setAmberIntensity(am);
            loadCurrentSettings();
            emit settingsChanged();
        });
        if (presetRow) presetRow->addWidget(btn);
    }

    cLay->addWidget(presetGroup);

    // ── Restaurar padrões ─────────────────────────────────────────────────────
    resetButton = new QPushButton("🔄 Restaurar Padrões", content);
    resetButton->setStyleSheet(
        "QPushButton{background:#8B0000;color:white;padding:12px;border-radius:6px;font-size:14px;font-weight:bold;}"
        "QPushButton:pressed{background:#A00000;}");
    connect(resetButton, &QPushButton::clicked, this, &SettingsScreen::onResetDefaults);
    cLay->addWidget(resetButton);

    cLay->addStretch();
    scroll->setWidget(content);
    mainLay->addWidget(scroll, 1);
}

void SettingsScreen::loadCurrentSettings()
{
    QColor mc = settingsManager->getMenuColor();
    QColor bc = settingsManager->getBgColor();
    currentColorLabel->setStyleSheet(
        QString("background:%1;border-radius:4px;border:2px solid #888;").arg(mc.name()));
    currentBgLabel->setStyleSheet(
        QString("background:%1;border-radius:4px;border:2px solid #888;").arg(bc.name()));
    nightModeCheck->setChecked(settingsManager->getNightMode());
    sepiaCheck->setChecked(settingsManager->getSepiaEnabled());
    brightnessSlider->setValue(settingsManager->getBrightness());
    amberSlider->setValue(settingsManager->getAmberIntensity());
}

void SettingsScreen::setMenuColor(const QColor& color)
{
    topBar->setStyleSheet(QString("background-color:%1;").arg(color.name()));
}

void SettingsScreen::onColorSelected(const QColor& color)
{
    settingsManager->setMenuColor(color);
    currentColorLabel->setStyleSheet(
        QString("background:%1;border-radius:4px;border:2px solid #888;").arg(color.name()));
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
