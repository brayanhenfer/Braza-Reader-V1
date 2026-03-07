#include "sidebarmenu.h"
#include <QPainter>
#include <QPainterPath>
#include <QHBoxLayout>
#include <cmath>

// ── Ícones QPainter ──────────────────────────────────────────────────────────

QPixmap SidebarMenu::iconBooks(int n){
    const int S=22; QPixmap px(S,S); px.fill(Qt::transparent);
    QPainter p(&px); p.setRenderHint(QPainter::Antialiasing);
    const int gap=2, total_gap=gap*(n-1), w=(S-total_gap)/n, h=S-3;
    for(int i=0;i<n;i++){
        const int x=i*(w+gap);
        p.setPen(Qt::NoPen); p.setBrush(QColor(200,200,200));
        p.drawRoundedRect(x,2,w,h,2,2);
        p.setBrush(Qt::white);
        p.drawRoundedRect(x+1,2,w-2,h-2,2,2);
    }
    return px;
}

QPixmap SidebarMenu::iconStar(){
    const int S=22; QPixmap px(S,S); px.fill(Qt::transparent);
    QPainter p(&px); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen); p.setBrush(Qt::white);
    const double cx=S/2.0,cy=S/2.0,outerR=10.0,innerR=4.2;
    QPainterPath path;
    for(int i=0;i<10;i++){
        const double angle=i*M_PI/5-M_PI/2.0;
        const double r=(i%2==0)?outerR:innerR;
        const QPointF pt(cx+r*std::cos(angle),cy+r*std::sin(angle));
        if(i==0) path.moveTo(pt); else path.lineTo(pt);
    }
    path.closeSubpath(); p.drawPath(path);
    return px;
}

QPixmap SidebarMenu::iconGear(){
    const int S=22; QPixmap px(S,S); px.fill(Qt::transparent);
    QPainter p(&px); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen); p.setBrush(Qt::white);
    const double cx=S/2.0,cy=S/2.0,outerR=9.5,innerR=6.5,holeR=3.5;
    const int teeth=8;
    QPainterPath path;
    for(int i=0;i<teeth*2;i++){
        const double angle=i*M_PI/teeth;
        const double r=(i%2==0)?outerR:innerR;
        const QPointF pt(cx+r*std::cos(angle),cy+r*std::sin(angle));
        if(i==0) path.moveTo(pt); else path.lineTo(pt);
    }
    path.closeSubpath();
    QPainterPath hole; hole.addEllipse(QPointF(cx,cy),holeR,holeR);
    p.drawPath(path.subtracted(hole));
    return px;
}

QPixmap SidebarMenu::iconPage(){
    const int S=22,fold=6; QPixmap px(S,S); px.fill(Qt::transparent);
    QPainter p(&px); p.setRenderHint(QPainter::Antialiasing);
    QPainterPath body;
    body.moveTo(3,1); body.lineTo(S-3-fold,1); body.lineTo(S-3,1+fold);
    body.lineTo(S-3,S-1); body.lineTo(3,S-1); body.closeSubpath();
    p.setPen(Qt::NoPen); p.setBrush(Qt::white); p.drawPath(body);
    QPainterPath tri;
    tri.moveTo(S-3-fold,1); tri.lineTo(S-3-fold,1+fold); tri.lineTo(S-3,1+fold); tri.closeSubpath();
    p.setBrush(QColor(160,160,160)); p.drawPath(tri);
    p.setPen(QPen(QColor(140,140,140),1.2));
    for(int y=7;y<=S-5;y+=4) p.drawLine(6,y,S-6,y);
    return px;
}

QPixmap SidebarMenu::iconInfo(){
    const int S=22; QPixmap px(S,S); px.fill(Qt::transparent);
    QPainter p(&px); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen); p.setBrush(Qt::white);
    p.drawEllipse(QRectF(2,2,S-4,S-4));
    p.setBrush(QColor(43,43,43));
    p.drawEllipse(QRectF(9.5,4.5,3,3));
    p.drawRoundedRect(QRectF(8.5,9,5,9),2,2);
    return px;
}

// ── Botão ────────────────────────────────────────────────────────────────────

QPushButton* SidebarMenu::makeButton(const QPixmap& icon, const QString& text){
    auto* btn=new QPushButton(this);
    btn->setFixedHeight(52);

    auto* inner=new QWidget(btn);
    inner->setAttribute(Qt::WA_TransparentForMouseEvents);
    inner->setStyleSheet("background:transparent;");
    auto* l=new QHBoxLayout(inner); l->setContentsMargins(16,0,16,0); l->setSpacing(14);
    auto* iconLbl=new QLabel(inner); iconLbl->setPixmap(icon); iconLbl->setFixedSize(22,22);
    iconLbl->setStyleSheet("background:transparent;");
    auto* txtLbl=new QLabel(text,inner); txtLbl->setStyleSheet("color:white;font-size:15px;background:transparent;");
    l->addWidget(iconLbl); l->addWidget(txtLbl); l->addStretch();

    auto* btnL=new QHBoxLayout(btn); btnL->setContentsMargins(0,0,0,0); btnL->addWidget(inner);

    btn->setStyleSheet(
        "QPushButton{background:transparent;border:none;border-bottom:1px solid rgba(255,255,255,0.08);}"
        "QPushButton:pressed{background:rgba(255,255,255,0.15);}"
        "QPushButton:hover{background:rgba(255,255,255,0.08);}");
    return btn;
}

// ── Setup ────────────────────────────────────────────────────────────────────

SidebarMenu::SidebarMenu(QWidget* parent):QWidget(parent){ setupUI(); }
SidebarMenu::~SidebarMenu()=default;

void SidebarMenu::setupUI(){
    setFixedWidth(230);
    // Sidebar: fundo cinza escuro — mesmo tom do app
    setStyleSheet("background-color:#252525;");

    mainLayout=new QVBoxLayout(this); mainLayout->setContentsMargins(0,0,0,0); mainLayout->setSpacing(0);

    // Cabeçalho com logo
    headerLabel=new QLabel(this); headerLabel->setFixedHeight(96); headerLabel->setAlignment(Qt::AlignCenter);
    headerLabel->setStyleSheet("background:rgba(0,0,0,0.30);");
    QPixmap lp(":/logo.png");
    if(!lp.isNull())
        headerLabel->setPixmap(lp.scaled(190, 72, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else{ headerLabel->setText("BrazaReader");
          headerLabel->setStyleSheet("color:white;font-size:22px;font-weight:bold;background:rgba(0,0,0,0.30);"); }
    mainLayout->addWidget(headerLabel);

    auto* sep=new QWidget(this); sep->setFixedHeight(1); sep->setStyleSheet("background:rgba(255,255,255,0.12);");
    mainLayout->addWidget(sep);

    auto* libBtn=makeButton(iconBooks(3),"Biblioteca");
    connect(libBtn,&QPushButton::clicked,this,&SidebarMenu::libraryClicked); mainLayout->addWidget(libBtn);
    auto* favBtn=makeButton(iconStar(),"Favoritos");
    connect(favBtn,&QPushButton::clicked,this,&SidebarMenu::favoritesClicked); mainLayout->addWidget(favBtn);
    auto* colBtn=makeButton(iconBooks(1),"Coleções");
    connect(colBtn,&QPushButton::clicked,this,&SidebarMenu::collectionsClicked); mainLayout->addWidget(colBtn);
    auto* setBtn=makeButton(iconGear(),"Configurações");
    connect(setBtn,&QPushButton::clicked,this,&SidebarMenu::settingsClicked); mainLayout->addWidget(setBtn);
    auto* terBtn=makeButton(iconPage(),"Termos de Uso");
    connect(terBtn,&QPushButton::clicked,this,&SidebarMenu::termsClicked); mainLayout->addWidget(terBtn);
    auto* abtBtn=makeButton(iconInfo(),"Sobre");
    connect(abtBtn,&QPushButton::clicked,this,&SidebarMenu::aboutClicked); mainLayout->addWidget(abtBtn);

    mainLayout->addStretch();

    auto* footer=new QWidget(this); footer->setFixedHeight(38);
    footer->setStyleSheet("background:rgba(0,0,0,0.25);");
    auto* fl=new QHBoxLayout(footer); fl->setContentsMargins(16,0,16,0);
    auto* vLbl=new QLabel("v1.0.0",footer); vLbl->setStyleSheet("color:rgba(255,255,255,0.35);font-size:11px;");
    fl->addWidget(vLbl,0,Qt::AlignLeft|Qt::AlignVCenter);
    mainLayout->addWidget(footer);
}
