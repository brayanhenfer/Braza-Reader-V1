#include "termsscreen.h"
#include "topbar_helper.h"
#include <QHBoxLayout>
#include <QScrollArea>

TermsScreen::TermsScreen(QWidget* parent) : QWidget(parent) { setupUI(); }
TermsScreen::~TermsScreen() = default;

void TermsScreen::applyMenuColor(const QColor& c) { TopBarHelper::setColor(topBar, c); }
void TermsScreen::applyBgColor(const QColor&) {}

void TermsScreen::setupUI()
{
    QVBoxLayout* ml = new QVBoxLayout(this);
    ml->setContentsMargins(0,0,0,0); ml->setSpacing(0);

    QPushButton* menuBtn = new QPushButton(QString::fromUtf8("☰"), this);
    menuBtn->setFixedSize(40,40);
    menuBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuBtn,&QPushButton::clicked,this,&TermsScreen::menuClicked);
    topBar = TopBarHelper::create(this, menuBtn);
    ml->addWidget(topBar);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;background:#242424;}");

    QWidget* content = new QWidget();
    content->setStyleSheet("background:#242424;");
    QVBoxLayout* cl = new QVBoxLayout(content);
    cl->setContentsMargins(28,22,28,22); cl->setSpacing(14);

    auto mkTitle = [&](const QString& t){
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
        l->setWordWrap(true); cl->addWidget(l);
    };
    auto mkBody = [&](const QString& t){
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet("color:#aaa;font-size:13px;");
        l->setWordWrap(true); cl->addWidget(l);
    };

    mkTitle(QString::fromUtf8("Termos de Uso \xe2\x80\x94 BrazaReader"));
    mkBody(QString::fromUtf8("\xc3\x9altima atualiza\xc3\xa7\xc3\xa3o: 2025"));

    mkTitle(QString::fromUtf8("1. Aceita\xc3\xa7\xc3\xa3o dos Termos"));
    mkBody(QString::fromUtf8(
        "Ao utilizar o BrazaReader, voc\xc3\xaa concorda com estes Termos de Uso. "
        "Se n\xc3\xa3o concordar com qualquer parte, n\xc3\xa3o utilize o aplicativo."));

    mkTitle(QString::fromUtf8("2. Uso do Aplicativo"));
    mkBody(QString::fromUtf8(
        "O BrazaReader \xc3\xa9 um leitor de PDF pessoal. Voc\xc3\xaa \xc3\xa9 respons\xc3\xa1vel "
        "pelo conte\xc3\xbado dos arquivos que abre. N\xc3\xa3o distribua arquivos protegidos "
        "por direitos autorais sem autoriza\xc3\xa7\xc3\xa3o do titular."));

    mkTitle(QString::fromUtf8("3. Dados e Privacidade"));
    mkBody(QString::fromUtf8(
        "O BrazaReader armazena dados localmente em seu dispositivo: "
        "progresso de leitura, anota\xc3\xa7\xc3\xb5""es, favoritos e configura\xc3\xa7\xc3\xb5""es. "
        "Nenhum dado \xc3\xa9 enviado a servidores externos."));

    mkTitle(QString::fromUtf8("4. Propriedade Intelectual"));
    mkBody(QString::fromUtf8(
        "O c\xc3\xb3""digo-fonte, design e marca BrazaReader s\xc3\xa3o propriedade de seus "
        "criadores. \xc3\x89 vedada a reprodu\xc3\xa7\xc3\xa3o, distribui\xc3\xa7\xc3\xa3o "
        "ou modifica\xc3\xa7\xc3\xa3o sem autoriza\xc3\xa7\xc3\xa3o expressa."));

    mkTitle(QString::fromUtf8("5. Limita\xc3\xa7\xc3\xa3o de Responsabilidade"));
    mkBody(QString::fromUtf8(
        "O aplicativo \xc3\xa9 fornecido 'como est\xc3\xa1', sem garantias de qualquer tipo. "
        "Os criadores n\xc3\xa3o se responsabilizam por perda de dados ou danos decorrentes do uso."));

    mkTitle(QString::fromUtf8("6. Altera\xc3\xa7\xc3\xb5""es"));
    mkBody(QString::fromUtf8(
        "Estes termos podem ser atualizados a qualquer momento. "
        "O uso continuado do aplicativo ap\xc3\xb3s altera\xc3\xa7\xc3\xb5""es "
        "implica aceita\xc3\xa7\xc3\xa3o dos novos termos."));

    cl->addStretch();
    scroll->setWidget(content);
    ml->addWidget(scroll,1);
}
