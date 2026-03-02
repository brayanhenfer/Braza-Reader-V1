#include "settingsscreen.h"
#include "topbar_helper.h"
#include "../storage/settingsmanager.h"

#include <QHBoxLayout>
#include <QColorDialog>
#include <QGroupBox>
#include <QScrollArea>
#include <cstdlib>

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

    // ── Topbar com logo ───────────────────────────────────────────────────────
    menuBtn = new QPushButton("☰");
    menuBtn->setFixedSize(40, 40);
    menuBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuBtn, &QPushButton::clicked, this, &SettingsScreen::menuClicked);

    topBar = TopBarHelper::create(this, menuBtn);
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

    // Helper para criar grupos
    auto makeGroup = [&](const QString& title) -> QGroupBox* {
        QGroupBox* g = new QGroupBox(title, content);
        g->setStyleSheet(
            "QGroupBox{color:white;font-size:14px;font-weight:bold;"
            "border:1px solid #555;border-radius:8px;margin-top:10px;padding-top:15px;}"
            "QGroupBox::title{subcontrol-origin:margin;left:15px;padding:0 5px;}");
        return g;
    };
    // Helper para criar botão de cor com preview
    auto makeColorRow = [&](QWidget* parent, const QString& btnText,
                             QLabel*& preview, const QColor& initial,
                             std::function<void(QColor)> onPick) -> QHBoxLayout*
    {
        QHBoxLayout* row = new QHBoxLayout();
        preview = new QLabel(parent);
        preview->setFixedSize(36, 36);
        preview->setStyleSheet(
            QString("background:%1;border-radius:4px;border:2px solid #888;").arg(initial.name()));

        QPushButton* btn = new QPushButton(btnText, parent);
        btn->setStyleSheet(
            "QPushButton{background:#444;color:white;padding:10px 16px;border-radius:6px;font-size:13px;}"
            "QPushButton:pressed{background:#555;}");
        connect(btn, &QPushButton::clicked, this, [onPick, initial, preview]() {
            QColor c = QColorDialog::getColor(initial, nullptr, "Escolher Cor");
            if (c.isValid()) {
                onPick(c);
                preview->setStyleSheet(
                    QString("background:%1;border-radius:4px;border:2px solid #888;").arg(c.name()));
            }
        });
        row->addWidget(preview);
        row->addWidget(btn, 1);
        return row;
    };

    // ── Cor do Menu ───────────────────────────────────────────────────────────
    {
        QGroupBox* g = makeGroup("Cor do Menu (Topbar)");
        QVBoxLayout* gl = new QVBoxLayout(g);
        gl->addLayout(makeColorRow(g, "Escolher Cor do Menu",
            menuColorPreview, settingsManager->getMenuColor(),
            [this](QColor c) {
                settingsManager->setMenuColor(c);
                emit settingsChanged();
            }));
        cLay->addWidget(g);
    }

    // ── Cor de Fundo do App ───────────────────────────────────────────────────
    // FIX: esta cor afeta o fundo externo do app (o cinza escuro que aparece por baixo)
    {
        QGroupBox* g = makeGroup("Cor de Fundo do App");
        QLabel* hint = new QLabel("A cor que aparece atrás de todo o conteúdo.", g);
        hint->setStyleSheet("color:#aaa;font-size:12px;");
        QVBoxLayout* gl = new QVBoxLayout(g);
        gl->addWidget(hint);
        gl->addLayout(makeColorRow(g, "Escolher Cor de Fundo",
            bgColorPreview, settingsManager->getBgColor(),
            [this](QColor c) {
                settingsManager->setBgColor(c);
                emit settingsChanged();
            }));
        cLay->addWidget(g);
    }

    // ── Cor de Janela ─────────────────────────────────────────────────────────
    // NOVO: cor dos cards, grades e painéis internos
    {
        QGroupBox* g = makeGroup("Cor de Janela");
        QLabel* hint = new QLabel("A cor dos cards, grade de livros e painéis internos.", g);
        hint->setStyleSheet("color:#aaa;font-size:12px;");
        QVBoxLayout* gl = new QVBoxLayout(g);
        gl->addWidget(hint);
        gl->addLayout(makeColorRow(g, "Escolher Cor de Janela",
            winColorPreview, settingsManager->getWindowColor(),
            [this](QColor c) {
                settingsManager->setWindowColor(c);
                emit settingsChanged();
            }));
        cLay->addWidget(g);
    }

    // ── Brilho ────────────────────────────────────────────────────────────────
    {
        QGroupBox* g = makeGroup("Brilho da Tela");
        QVBoxLayout* gl = new QVBoxLayout(g);
        brightnessLabel = new QLabel("100%", g);
        brightnessLabel->setStyleSheet("color:white;font-size:13px;");
        brightnessSlider = new QSlider(Qt::Horizontal, g);
        brightnessSlider->setRange(20, 100);
        brightnessSlider->setValue(100);
        brightnessSlider->setStyleSheet(
            "QSlider::groove:horizontal{background:#444;height:6px;border-radius:3px;}"
            "QSlider::handle:horizontal{background:#4CAF50;width:22px;height:22px;margin:-8px 0;border-radius:11px;}"
            "QSlider::sub-page:horizontal{background:#1e6432;border-radius:3px;}");
        connect(brightnessSlider, &QSlider::valueChanged, this, [this](int v) {
            brightnessLabel->setText(QString("%1%").arg(v));
            settingsManager->setBrightness(v);
            emit settingsChanged();
        });
        gl->addWidget(brightnessSlider);
        gl->addWidget(brightnessLabel);
        cLay->addWidget(g);
    }

    // ── Temperatura de Cor (Âmbar) ────────────────────────────────────────────
    // FIX: agora realmente aplica via ReaderScreen::setAmberIntensity → PageWidget::paintEvent
    {
        QGroupBox* g = makeGroup("Temperatura de Cor — Filtro Âmbar");
        QLabel* hint = new QLabel(
            "Aplica um filtro laranja-quente durante a leitura. Funciona ao abrir um livro.", g);
        hint->setStyleSheet("color:#aaa;font-size:12px;");
        hint->setWordWrap(true);
        QVBoxLayout* gl = new QVBoxLayout(g);
        gl->addWidget(hint);
        amberLabel = new QLabel("Desligado", g);
        amberLabel->setStyleSheet("color:#FFA040;font-size:13px;");
        amberSlider = new QSlider(Qt::Horizontal, g);
        amberSlider->setRange(0, 100);
        amberSlider->setValue(0);
        amberSlider->setStyleSheet(
            "QSlider::groove:horizontal{background:#444;height:6px;border-radius:3px;}"
            "QSlider::handle:horizontal{background:#FF9500;width:22px;height:22px;margin:-8px 0;border-radius:11px;}"
            "QSlider::sub-page:horizontal{background:#FF6600;border-radius:3px;}");
        connect(amberSlider, &QSlider::valueChanged, this, [this](int v) {
            amberLabel->setText(v == 0 ? "Desligado" : QString("Intensidade %1%").arg(v));
            settingsManager->setAmberIntensity(v);
            emit settingsChanged(); // MainWindow propaga para ReaderScreen
        });
        gl->addWidget(amberSlider);
        gl->addWidget(amberLabel);
        cLay->addWidget(g);
    }

    // ── Aparência ─────────────────────────────────────────────────────────────
    {
        QGroupBox* g = makeGroup("Aparência");
        QVBoxLayout* gl = new QVBoxLayout(g);
        nightModeCheck = new QCheckBox("🌙 Modo Noturno", g);
        nightModeCheck->setStyleSheet("QCheckBox{color:white;font-size:14px;spacing:10px;}");
        connect(nightModeCheck, &QCheckBox::toggled, this, [this](bool on) {
            settingsManager->setNightMode(on);
            emit settingsChanged();
        });
        sepiaCheck = new QCheckBox("📜 Modo Sépia", g);
        sepiaCheck->setStyleSheet("QCheckBox{color:white;font-size:14px;spacing:10px;}");
        connect(sepiaCheck, &QCheckBox::toggled, this, [this](bool on) {
            settingsManager->setSepiaEnabled(on);
            emit settingsChanged();
        });
        gl->addWidget(nightModeCheck);
        gl->addWidget(sepiaCheck);
        cLay->addWidget(g);
    }

    // ── Orientação ────────────────────────────────────────────────────────────
    {
        QGroupBox* g = makeGroup("Orientação do Ecrã");
        QHBoxLayout* gl = new QHBoxLayout(g);
        auto makeOrientBtn = [&](const QString& label, const QString& arg) {
            QPushButton* b = new QPushButton(label, g);
            b->setStyleSheet(
                "QPushButton{background:#444;color:white;padding:10px;border-radius:6px;}"
                "QPushButton:pressed{background:#1e6432;}");
            connect(b, &QPushButton::clicked, this, [arg]() {
                QString cmd = QString(
                    "xrandr --output HDMI-1 --rotate %1 2>/dev/null || "
                    "xrandr --output DSI-1  --rotate %1 2>/dev/null || "
                    "xrandr --output LVDS-1 --rotate %1 2>/dev/null").arg(arg);
                system(cmd.toStdString().c_str());
            });
            return b;
        };
        gl->addWidget(makeOrientBtn("↕ Retrato",  "normal"));
        gl->addWidget(makeOrientBtn("↔ Paisagem", "left"));
        gl->addWidget(makeOrientBtn("↕ Inv.",     "inverted"));
        gl->addWidget(makeOrientBtn("↔ Pais.2",  "right"));
        cLay->addWidget(g);
    }

    // ── Presets ───────────────────────────────────────────────────────────────
    {
        QGroupBox* g = makeGroup("Presets de Aparência");
        QVBoxLayout* gl = new QVBoxLayout(g);

        struct Preset { QString label, menu, bg, win; bool night, sepia; int amber; };
        QList<Preset> presets = {
            {"🌿 Floresta",    "#1e6432", "#1a1a1a", "#2b2b2b", false, false,  0},
            {"🌊 Oceano",      "#1a4a7a", "#0e1a2a", "#1e2a3a", false, false,  0},
            {"🌙 Noturno",     "#111111", "#080808", "#151515", true,  false,  0},
            {"📜 Sépia",       "#7a5c2e", "#1a0e00", "#2a1e10", false, true,  25},
            {"🕯️  Vela",        "#8B4513", "#0a0500", "#1a0e00", true,  false, 60},
            {"🖤 Preto Total", "#000000", "#000000", "#0a0a0a", true,  false,  0},
        };

        QHBoxLayout* row = nullptr;
        for (int i = 0; i < presets.size(); i++) {
            if (i % 2 == 0) { row = new QHBoxLayout(); gl->addLayout(row); }
            const Preset& p = presets[i];
            QPushButton* btn = new QPushButton(p.label, g);
            btn->setStyleSheet(
                QString("QPushButton{background:%1;color:white;padding:10px;"
                        "border-radius:6px;font-size:12px;border:2px solid #555;}"
                        "QPushButton:pressed{border-color:white;}").arg(p.menu));
            QString mc=p.menu, bc=p.bg, wc=p.win;
            bool nt=p.night, sp=p.sepia; int am=p.amber;
            connect(btn, &QPushButton::clicked, this, [this,mc,bc,wc,nt,sp,am]() {
                settingsManager->setMenuColor(QColor(mc));
                settingsManager->setBgColor(QColor(bc));
                settingsManager->setWindowColor(QColor(wc));
                settingsManager->setNightMode(nt);
                settingsManager->setSepiaEnabled(sp);
                settingsManager->setAmberIntensity(am);
                loadCurrentSettings();
                emit settingsChanged();
            });
            if (row) row->addWidget(btn);
        }
        cLay->addWidget(g);
    }

    // ── Restaurar padrões ─────────────────────────────────────────────────────
    QPushButton* resetBtn = new QPushButton("🔄 Restaurar Padrões", content);
    resetBtn->setStyleSheet(
        "QPushButton{background:#8B0000;color:white;padding:12px;border-radius:6px;"
        "font-size:14px;font-weight:bold;}"
        "QPushButton:pressed{background:#A00000;}");
    connect(resetBtn, &QPushButton::clicked, this, [this]() {
        settingsManager->resetToDefaults();
        loadCurrentSettings();
        emit settingsChanged();
    });
    cLay->addWidget(resetBtn);
    cLay->addStretch();

    scroll->setWidget(content);
    mainLay->addWidget(scroll, 1);
}

void SettingsScreen::loadCurrentSettings()
{
    auto setPreview = [](QLabel* lbl, const QColor& c) {
        if (lbl) lbl->setStyleSheet(
            QString("background:%1;border-radius:4px;border:2px solid #888;").arg(c.name()));
    };
    setPreview(menuColorPreview, settingsManager->getMenuColor());
    setPreview(bgColorPreview,   settingsManager->getBgColor());
    setPreview(winColorPreview,  settingsManager->getWindowColor());
    nightModeCheck->setChecked(settingsManager->getNightMode());
    sepiaCheck->setChecked(settingsManager->getSepiaEnabled());
    brightnessSlider->setValue(settingsManager->getBrightness());
    amberSlider->setValue(settingsManager->getAmberIntensity());
    int am = settingsManager->getAmberIntensity();
    amberLabel->setText(am == 0 ? "Desligado" : QString("Intensidade %1%").arg(am));
    brightnessLabel->setText(QString("%1%").arg(settingsManager->getBrightness()));
}

void SettingsScreen::setMenuColor(const QColor& color)
{
    TopBarHelper::setColor(topBar, color);
}
