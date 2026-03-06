#include "sidebarmenu.h"
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <QPixmap>
#include <cmath>

// ── Ícones desenhados em QPixmap 22×22 ───────────────────────────────────────

QPixmap SidebarMenu::iconBooks(int n)
{
    const int S = 22;
    QPixmap px(S, S); px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    const int gap = 2;
    const int total_gap = gap * (n - 1);
    const int w = (S - total_gap) / n;
    const int h = S - 3;
    for (int i = 0; i < n; i++) {
        const int x = i * (w + gap);
        // Spine of book (rounded top)
        p.drawRoundedRect(x, 2, w, h, 2, 2);
        // Slight shade on top (pages)
        p.setBrush(QColor(200, 200, 200));
        p.drawRoundedRect(x + 1, 2, w - 1, 3, 1, 1);
        p.setBrush(Qt::white);
    }
    return px;
}

QPixmap SidebarMenu::iconStar()
{
    const int S = 22;
    QPixmap px(S, S); px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 215, 0));

    const double cx = S / 2.0, cy = S / 2.0;
    const double outerR = 10.0, innerR = 4.2;
    const int points = 5;
    QPainterPath path;
    for (int i = 0; i < points * 2; i++) {
        const double angle = i * M_PI / points - M_PI / 2.0;
        const double r = (i % 2 == 0) ? outerR : innerR;
        const QPointF pt(cx + r * std::cos(angle), cy + r * std::sin(angle));
        if (i == 0) path.moveTo(pt); else path.lineTo(pt);
    }
    path.closeSubpath();
    p.drawPath(path);
    return px;
}

QPixmap SidebarMenu::iconGear()
{
    const int S = 22;
    QPixmap px(S, S); px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);

    const double cx = S / 2.0, cy = S / 2.0;
    const int teeth = 8;
    const double outerR = 9.5, innerR = 6.5, holeR = 3.5;
    QPainterPath path;
    for (int i = 0; i < teeth * 2; i++) {
        const double angle = i * M_PI / teeth;
        const double r = (i % 2 == 0) ? outerR : innerR;
        const QPointF pt(cx + r * std::cos(angle), cy + r * std::sin(angle));
        if (i == 0) path.moveTo(pt); else path.lineTo(pt);
    }
    path.closeSubpath();
    // Cut center hole
    QPainterPath hole;
    hole.addEllipse(QPointF(cx, cy), holeR, holeR);
    p.drawPath(path.subtracted(hole));
    return px;
}

QPixmap SidebarMenu::iconPage()
{
    const int S = 22;
    QPixmap px(S, S); px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);

    const int fold = 6;
    // Body
    QPainterPath body;
    body.moveTo(3, 1);
    body.lineTo(S - 3 - fold, 1);
    body.lineTo(S - 3, 1 + fold);
    body.lineTo(S - 3, S - 1);
    body.lineTo(3, S - 1);
    body.closeSubpath();
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawPath(body);

    // Fold triangle
    QPainterPath tri;
    tri.moveTo(S - 3 - fold, 1);
    tri.lineTo(S - 3 - fold, 1 + fold);
    tri.lineTo(S - 3, 1 + fold);
    tri.closeSubpath();
    p.setBrush(QColor(160, 160, 160));
    p.drawPath(tri);

    // Lines (text)
    p.setPen(QPen(QColor(140, 140, 140), 1.2));
    for (int y = 7; y <= S - 5; y += 4)
        p.drawLine(6, y, S - 6, y);

    return px;
}

QPixmap SidebarMenu::iconInfo()
{
    const int S = 22;
    QPixmap px(S, S); px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::white);
    p.drawEllipse(QRectF(2, 2, S - 4, S - 4));
    // "i" dot
    p.setBrush(QColor(30, 100, 50));
    p.drawEllipse(QRectF(9.5, 4.5, 3, 3));
    // "i" bar
    p.drawRoundedRect(QRectF(8.5, 9, 5, 9), 2, 2);
    return px;
}

// ── Botão padrão da sidebar ───────────────────────────────────────────────────

QPushButton* SidebarMenu::makeButton(const QPixmap& icon, const QString& text)
{
    QPushButton* btn = new QPushButton(this);
    btn->setFixedHeight(52);

    // Layout interno: ícone + texto
    QWidget* inner = new QWidget(btn);
    QHBoxLayout* l = new QHBoxLayout(inner);
    l->setContentsMargins(16, 0, 16, 0);
    l->setSpacing(14);

    QLabel* iconLbl = new QLabel(inner);
    iconLbl->setPixmap(icon);
    iconLbl->setFixedSize(22, 22);

    QLabel* txtLbl = new QLabel(text, inner);
    txtLbl->setStyleSheet("color:white;font-size:15px;background:transparent;");

    l->addWidget(iconLbl);
    l->addWidget(txtLbl);
    l->addStretch();

    // Faz o inner ocupar o botão inteiro
    inner->setAttribute(Qt::WA_TransparentForMouseEvents);
    QHBoxLayout* btnL = new QHBoxLayout(btn);
    btnL->setContentsMargins(0, 0, 0, 0);
    btnL->addWidget(inner);

    btn->setStyleSheet(
        "QPushButton {"
        "  background:transparent;"
        "  border:none;"
        "  border-bottom:1px solid rgba(255,255,255,0.08);"
        "  text-align:left;"
        "}"
        "QPushButton:pressed { background:rgba(255,255,255,0.18); }"
        "QPushButton:hover   { background:rgba(255,255,255,0.09); }"
    );
    return btn;
}

// ── Setup ─────────────────────────────────────────────────────────────────────

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
    headerLabel->setFixedHeight(76);
    headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet("background:rgba(0,0,0,0.22);");
    QPixmap lp(":/logo.png");
    if (!lp.isNull())
        headerLabel->setPixmap(lp.scaled(150, 54, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else {
        headerLabel->setText("BrazaReader");
        headerLabel->setStyleSheet(
            "color:white;font-size:20px;font-weight:bold;background:rgba(0,0,0,0.22);");
    }
    mainLayout->addWidget(headerLabel);

    QWidget* sep = new QWidget(this);
    sep->setFixedHeight(1);
    sep->setStyleSheet("background:rgba(255,255,255,0.15);");
    mainLayout->addWidget(sep);

    // ── Botões ────────────────────────────────────────────────────────────
    auto* libBtn = makeButton(iconBooks(3), "Biblioteca");
    connect(libBtn, &QPushButton::clicked, this, &SidebarMenu::libraryClicked);
    mainLayout->addWidget(libBtn);

    auto* favBtn = makeButton(iconStar(), "Favoritos");
    connect(favBtn, &QPushButton::clicked, this, &SidebarMenu::favoritesClicked);
    mainLayout->addWidget(favBtn);

    auto* colBtn = makeButton(iconBooks(1), "Coleções");
    connect(colBtn, &QPushButton::clicked, this, &SidebarMenu::collectionsClicked);
    mainLayout->addWidget(colBtn);

    auto* setBtn = makeButton(iconGear(), "Configurações");
    connect(setBtn, &QPushButton::clicked, this, &SidebarMenu::settingsClicked);
    mainLayout->addWidget(setBtn);

    auto* terBtn = makeButton(iconPage(), "Termos de Uso");
    connect(terBtn, &QPushButton::clicked, this, &SidebarMenu::termsClicked);
    mainLayout->addWidget(terBtn);

    auto* abtBtn = makeButton(iconInfo(), "Sobre");
    connect(abtBtn, &QPushButton::clicked, this, &SidebarMenu::aboutClicked);
    mainLayout->addWidget(abtBtn);

    mainLayout->addStretch();

    // ── Rodapé ────────────────────────────────────────────────────────────
    QWidget* footer = new QWidget(this);
    footer->setFixedHeight(38);
    footer->setStyleSheet("background:rgba(0,0,0,0.2);");
    QHBoxLayout* fl = new QHBoxLayout(footer);
    fl->setContentsMargins(16, 0, 16, 0);
    QLabel* vLbl = new QLabel("v1.0.0", footer);
    vLbl->setStyleSheet("color:rgba(255,255,255,0.4);font-size:11px;");
    fl->addWidget(vLbl, 0, Qt::AlignLeft | Qt::AlignVCenter);
    mainLayout->addWidget(footer);
}
