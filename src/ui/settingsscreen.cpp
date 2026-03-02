#include "settingsscreen.h"
#include "topbar_helper.h"
#include "../storage/settingsmanager.h"

#include <QHBoxLayout>
#include <QColorDialog>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QProcess>

SettingsScreen::SettingsScreen(QWidget* parent)
    : QWidget(parent)
    , settingsManager(std::make_unique<SettingsManager>())
{
    setupUI();
    loadCurrentSettings();
}

SettingsScreen::~SettingsScreen() = default;

// ── Helpers internos ──────────────────────────────────────────────────────────

static QGroupBox* makeGroup(const QString& title, QWidget* parent)
{
    auto* g = new QGroupBox(title, parent);
    g->setStyleSheet(
        "QGroupBox{color:white;font-size:14px;font-weight:bold;"
        "border:1px solid #555;border-radius:8px;margin-top:10px;padding-top:14px;}"
        "QGroupBox::title{subcontrol-origin:margin;left:14px;padding:0 5px;}");
    return g;
}

static QPushButton* makeColorBtn(const QString& text, QWidget* parent)
{
    auto* b = new QPushButton(text, parent);
    b->setStyleSheet(
        "QPushButton{background:#444;color:white;padding:10px 16px;"
        "border-radius:6px;font-size:13px;}"
        "QPushButton:pressed{background:#666;}");
    return b;
}

// Detecta o output de vídeo ativo via xrandr e retorna o nome (ex: "HDMI-1")
static QString detectXrandrOutput()
{
    QProcess p;
    p.start("xrandr", {"--query"});
    p.waitForFinished(2000);
    QString out = p.readAllStandardOutput();
    // Procura primeira linha com " connected"
    for (const QString& line : out.split('\n')) {
        if (line.contains(" connected")) {
            return line.split(' ').first();
        }
    }
    // Fallback: lista comum de outputs do RPi
    return "";
}

// Aplica rotação via xrandr (X11) ou wlr-randr (Wayland)
static void applyRotation(const QString& xrandrArg, const QString& wlrArg)
{
    // 1. Tenta wlr-randr (Wayland/kmsdrm moderno)
    {
        QProcess p;
        p.start("wlr-randr", {"--output", "HDMI-A-1", "--transform", wlrArg});
        if (p.waitForFinished(1500) && p.exitCode() == 0) return;
    }

    // 2. Tenta xrandr com detecção automática do output
    QString output = detectXrandrOutput();
    if (!output.isEmpty()) {
        QProcess p;
        p.start("xrandr", {"--output", output, "--rotate", xrandrArg});
        if (p.waitForFinished(2000) && p.exitCode() == 0) return;
    }

    // 3. Tenta outputs comuns do RPi manualmente
    QStringList outputs = {"HDMI-1","HDMI-2","DSI-1","LVDS-1","DPI-1","Composite-1"};
    for (const QString& o : outputs) {
        QProcess p;
        p.start("xrandr", {"--output", o, "--rotate", xrandrArg});
        if (p.waitForFinished(1500) && p.exitCode() == 0) return;
    }

    // 4. Último recurso: fbcon via /sys (funciona sem Xorg)
    // echo 1 > /sys/class/graphics/fbcon/rotate_all  (0=normal,1=CW,2=UD,3=CCW)
    QString fbRot = "0";
    if      (xrandrArg == "normal")   fbRot = "0";
    else if (xrandrArg == "right")    fbRot = "1";
    else if (xrandrArg == "inverted") fbRot = "2";
    else if (xrandrArg == "left")     fbRot = "3";

    QProcess p;
    p.start("bash", {"-c",
        QString("echo %1 > /sys/class/graphics/fbcon/rotate_all 2>/dev/null || "
                "echo %1 > /sys/class/graphics/fbcon/rotate 2>/dev/null")
        .arg(fbRot)});
    p.waitForFinished(1000);
}

// ── Setup UI ──────────────────────────────────────────────────────────────────

void SettingsScreen::setupUI()
{
    QVBoxLayout* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // Topbar
    menuBtn = new QPushButton("☰");
    menuBtn->setFixedSize(40, 40);
    menuBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuBtn, &QPushButton::clicked, this, &SettingsScreen::menuClicked);
    topBar = TopBarHelper::create(this, menuBtn);
    mainLay->addWidget(topBar);

    // ScrollArea com grupos
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;background:#2b2b2b;}");

    QWidget* content = new QWidget();
    content->setStyleSheet("background:#2b2b2b;");
    QVBoxLayout* cLay = new QVBoxLayout(content);
    cLay->setContentsMargins(16, 16, 16, 16);
    cLay->setSpacing(16);

    // ── Cor do Menu ───────────────────────────────────────────────────────────
    {
        auto* g  = makeGroup("Cor do Menu (Topbar)", content);
        auto* gl = new QHBoxLayout(g);
        menuColorPreview = new QLabel(g);
        menuColorPreview->setFixedSize(36, 36);
        auto* btn = makeColorBtn("Escolher Cor do Menu", g);
        connect(btn, &QPushButton::clicked, this, [this]() {
            QColor c = QColorDialog::getColor(settingsManager->getMenuColor(),
                                               this, "Cor do Menu");
            if (c.isValid()) {
                settingsManager->setMenuColor(c);
                menuColorPreview->setStyleSheet(
                    QString("background:%1;border-radius:4px;border:2px solid #888;").arg(c.name()));
                emit settingsChanged();
            }
        });
        gl->addWidget(menuColorPreview);
        gl->addWidget(btn, 1);
        cLay->addWidget(g);
    }

    // ── Cor de Fundo do App ───────────────────────────────────────────────────
    {
        auto* g  = makeGroup("Cor de Fundo do App", content);
        auto* gl = new QVBoxLayout(g);
        auto* hint = new QLabel("Cor que aparece atrás de todo o conteúdo.", g);
        hint->setStyleSheet("color:#aaa;font-size:12px;");
        gl->addWidget(hint);
        auto* row = new QHBoxLayout();
        bgColorPreview = new QLabel(g);
        bgColorPreview->setFixedSize(36, 36);
        auto* btn = makeColorBtn("Escolher Cor de Fundo", g);
        connect(btn, &QPushButton::clicked, this, [this]() {
            QColor c = QColorDialog::getColor(settingsManager->getBgColor(),
                                               this, "Cor de Fundo");
            if (c.isValid()) {
                settingsManager->setBgColor(c);
                bgColorPreview->setStyleSheet(
                    QString("background:%1;border-radius:4px;border:2px solid #888;").arg(c.name()));
                emit settingsChanged();
            }
        });
        row->addWidget(bgColorPreview);
        row->addWidget(btn, 1);
        gl->addLayout(row);
        cLay->addWidget(g);
    }

    // ── Cor de Janela ─────────────────────────────────────────────────────────
    {
        auto* g  = makeGroup("Cor de Janela", content);
        auto* gl = new QVBoxLayout(g);
        auto* hint = new QLabel("Cor dos cards, grade de livros e painéis internos.", g);
        hint->setStyleSheet("color:#aaa;font-size:12px;");
        gl->addWidget(hint);
        auto* row = new QHBoxLayout();
        winColorPreview = new QLabel(g);
        winColorPreview->setFixedSize(36, 36);
        auto* btn = makeColorBtn("Escolher Cor de Janela", g);
        connect(btn, &QPushButton::clicked, this, [this]() {
            QColor c = QColorDialog::getColor(settingsManager->getWindowColor(),
                                               this, "Cor de Janela");
            if (c.isValid()) {
                settingsManager->setWindowColor(c);
                winColorPreview->setStyleSheet(
                    QString("background:%1;border-radius:4px;border:2px solid #888;").arg(c.name()));
                emit settingsChanged();
            }
        });
        row->addWidget(winColorPreview);
        row->addWidget(btn, 1);
        gl->addLayout(row);
        cLay->addWidget(g);
    }

    // ── Brilho ────────────────────────────────────────────────────────────────
    {
        auto* g  = makeGroup("Brilho da Tela", content);
        auto* gl = new QVBoxLayout(g);
        brightnessLabel = new QLabel("100%", g);
        brightnessLabel->setStyleSheet("color:white;font-size:13px;");
        brightnessSlider = new QSlider(Qt::Horizontal, g);
        brightnessSlider->setRange(20, 100);
        brightnessSlider->setStyleSheet(
            "QSlider::groove:horizontal{background:#444;height:6px;border-radius:3px;}"
            "QSlider::handle:horizontal{background:#4CAF50;width:22px;height:22px;"
            "margin:-8px 0;border-radius:11px;}"
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
    // FIX: âmbar agora age na TELA TODA via overlay na MainWindow
    {
        auto* g  = makeGroup("Temperatura de Cor — Filtro Âmbar", content);
        auto* gl = new QVBoxLayout(g);
        auto* hint = new QLabel(
            "Filtro laranja-quente sobre toda a tela (menus e leitor).", g);
        hint->setStyleSheet("color:#aaa;font-size:12px;");
        hint->setWordWrap(true);
        gl->addWidget(hint);
        amberLabel = new QLabel("Desligado", g);
        amberLabel->setStyleSheet("color:#FFA040;font-size:13px;");
        amberSlider = new QSlider(Qt::Horizontal, g);
        amberSlider->setRange(0, 100);
        amberSlider->setStyleSheet(
            "QSlider::groove:horizontal{background:#444;height:6px;border-radius:3px;}"
            "QSlider::handle:horizontal{background:#FF9500;width:22px;height:22px;"
            "margin:-8px 0;border-radius:11px;}"
            "QSlider::sub-page:horizontal{background:#FF6600;border-radius:3px;}");
        connect(amberSlider, &QSlider::valueChanged, this, [this](int v) {
            amberLabel->setText(v == 0 ? "Desligado"
                                       : QString("Intensidade  %1%").arg(v));
            settingsManager->setAmberIntensity(v);
            emit settingsChanged();
        });
        gl->addWidget(amberSlider);
        gl->addWidget(amberLabel);
        cLay->addWidget(g);
    }

    // ── Aparência ─────────────────────────────────────────────────────────────
    {
        auto* g  = makeGroup("Aparência", content);
        auto* gl = new QVBoxLayout(g);
        nightModeCheck = new QCheckBox("🌙  Modo Noturno", g);
        nightModeCheck->setStyleSheet(
            "QCheckBox{color:white;font-size:14px;spacing:10px;}");
        sepiaCheck = new QCheckBox("📜  Modo Sépia", g);
        sepiaCheck->setStyleSheet(nightModeCheck->styleSheet());
        connect(nightModeCheck, &QCheckBox::toggled, this, [this](bool on) {
            settingsManager->setNightMode(on); emit settingsChanged();
        });
        connect(sepiaCheck, &QCheckBox::toggled, this, [this](bool on) {
            settingsManager->setSepiaEnabled(on); emit settingsChanged();
        });
        gl->addWidget(nightModeCheck);
        gl->addWidget(sepiaCheck);
        cLay->addWidget(g);
    }

    // ── Orientação do Ecrã ────────────────────────────────────────────────────
    // FIX: usa QProcess para xrandr/wlr-randr/fbcon com detecção automática de output
    {
        auto* g  = makeGroup("Orientação do Ecrã", content);
        auto* hint = new QLabel(
            "Aplica rotação via xrandr (X11), wlr-randr (Wayland) ou\n"
            "fbcon (framebuffer). Requer que o servidor gráfico suporte rotação.", g);
        hint->setStyleSheet("color:#aaa;font-size:12px;");
        hint->setWordWrap(true);
        auto* gl = new QVBoxLayout(g);
        gl->addWidget(hint);

        auto* row = new QHBoxLayout();
        struct OrientDef { QString label, xrandr, wlr; };
        QList<OrientDef> orients = {
            {"↕ Retrato",   "normal",   "normal"},
            {"↔ Paisagem",  "left",     "90"},
            {"↕ Invertido", "inverted", "180"},
            {"↔ Pais. 2",  "right",    "270"},
        };
        for (const auto& o : orients) {
            auto* btn = new QPushButton(o.label, g);
            btn->setStyleSheet(
                "QPushButton{background:#444;color:white;padding:10px 6px;"
                "border-radius:6px;font-size:12px;}"
                "QPushButton:pressed{background:#1e6432;}");
            QString xr = o.xrandr, wl = o.wlr;
            connect(btn, &QPushButton::clicked, this, [xr, wl]() {
                applyRotation(xr, wl);
            });
            row->addWidget(btn);
        }
        gl->addLayout(row);
        cLay->addWidget(g);
    }

    // ── Presets de aparência ──────────────────────────────────────────────────
    {
        auto* g  = makeGroup("Presets de Aparência", content);
        auto* gl = new QVBoxLayout(g);

        struct Preset {
            QString label, menu, bg, win;
            bool night, sepia;
            int amber;
        };
        QList<Preset> presets = {
            {"🌿 Floresta",    "#1e6432", "#1a1a1a", "#2b2b2b", false, false,  0},
            {"🌊 Oceano",      "#1a4a7a", "#0e1a2a", "#1e2a3a", false, false,  0},
            {"🌙 Noturno",     "#111111", "#080808", "#151515", true,  false,  0},
            {"📜 Sépia",       "#7a5c2e", "#1a0e00", "#2a1e10", false, true,  30},
            {"🕯 Vela",        "#8B4513", "#0a0500", "#1a0e00", true,  false, 60},
            {"🖤 Preto Total", "#000000", "#000000", "#0a0a0a", true,  false,  0},
        };

        QHBoxLayout* row = nullptr;
        for (int i = 0; i < presets.size(); i++) {
            if (i % 2 == 0) { row = new QHBoxLayout(); gl->addLayout(row); }
            const auto& p = presets[i];
            auto* btn = new QPushButton(p.label, g);
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
    auto* resetBtn = new QPushButton("🔄  Restaurar Padrões", content);
    resetBtn->setStyleSheet(
        "QPushButton{background:#8B0000;color:white;padding:12px;"
        "border-radius:6px;font-size:14px;font-weight:bold;}"
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

// ── Carrega valores salvos nos widgets ────────────────────────────────────────

void SettingsScreen::loadCurrentSettings()
{
    auto preview = [](QLabel* lbl, const QColor& c) {
        if (lbl) lbl->setStyleSheet(
            QString("background:%1;border-radius:4px;border:2px solid #888;")
            .arg(c.name()));
    };
    preview(menuColorPreview, settingsManager->getMenuColor());
    preview(bgColorPreview,   settingsManager->getBgColor());
    preview(winColorPreview,  settingsManager->getWindowColor());

    nightModeCheck->setChecked(settingsManager->getNightMode());
    sepiaCheck->setChecked(settingsManager->getSepiaEnabled());

    int br = settingsManager->getBrightness();
    brightnessSlider->setValue(br);
    brightnessLabel->setText(QString("%1%").arg(br));

    int am = settingsManager->getAmberIntensity();
    amberSlider->setValue(am);
    amberLabel->setText(am == 0 ? "Desligado"
                                : QString("Intensidade  %1%").arg(am));
}

void SettingsScreen::setMenuColor(const QColor& color)
{
    TopBarHelper::setColor(topBar, color);
}
