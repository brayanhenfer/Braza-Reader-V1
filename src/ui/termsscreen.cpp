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

    QPushButton* menuBtn = new QPushButton(QString::fromUtf8("\xe2\x98\xb0"), this);
    menuBtn->setFixedSize(40,40);
    menuBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuBtn,&QPushButton::clicked,this,&TermsScreen::menuClicked);
    topBar = TopBarHelper::create(this, menuBtn);
    ml->addWidget(topBar);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;background:#1e1e1e;}");

    QWidget* content = new QWidget();
    content->setStyleSheet("background:#1e1e1e;");
    QVBoxLayout* cl = new QVBoxLayout(content);
    cl->setContentsMargins(40, 32, 40, 40);
    cl->setSpacing(0);

    auto mkHeader = [&](const QString& t, const QString& sub) {
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet("color:#2ecc71;font-size:26px;font-weight:bold;");
        l->setWordWrap(true);
        cl->addWidget(l);
        QLabel* s = new QLabel(sub, content);
        s->setStyleSheet("color:#666;font-size:12px;margin-bottom:24px;");
        cl->addWidget(s);
        QWidget* div = new QWidget(content);
        div->setFixedHeight(2);
        div->setStyleSheet("background:#1e6432;margin-bottom:24px;");
        cl->addWidget(div);
    };

    auto mkTitle = [&](const QString& t) {
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet(
            "color:white;font-size:14px;font-weight:bold;"
            "margin-top:20px;margin-bottom:6px;"
            "border-left:3px solid #1e6432;padding-left:10px;");
        l->setWordWrap(true);
        cl->addWidget(l);
    };

    auto mkSub = [&](const QString& t) {
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet(
            "color:#ccc;font-size:13px;font-weight:bold;"
            "margin-top:12px;margin-bottom:4px;padding-left:4px;");
        l->setWordWrap(true);
        cl->addWidget(l);
    };

    auto mkBody = [&](const QString& t) {
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet(
            "color:#aaa;font-size:13px;line-height:1.6;"
            "margin-bottom:8px;padding-left:4px;");
        l->setWordWrap(true);
        cl->addWidget(l);
    };

    auto mkItem = [&](const QString& t) {
        QLabel* l = new QLabel(QString::fromUtf8("\xe2\x80\xa2  ") + t, content);
        l->setStyleSheet(
            "color:#999;font-size:13px;padding-left:16px;margin-bottom:4px;");
        l->setWordWrap(true);
        cl->addWidget(l);
    };

    // ── Conteúdo ─────────────────────────────────────────────────────────────

    mkHeader(
        "BrazaReader",
        QString::fromUtf8("Termos de Uso  \xe2\x80\x94  Brayan Henfer  \xe2\x80\x94  Vers\xc3\xa3o 1.0"));

    mkTitle(QString::fromUtf8("1. Aceita\xc3\xa7\xc3\xa3o dos Termos"));
    mkBody(QString::fromUtf8(
        "Ao instalar, acessar ou utilizar o BrazaReader, voc\xc3\xaa declara que leu, "
        "compreendeu e concorda com os presentes Termos de Uso. Caso n\xc3\xa3o concorde "
        "com alguma das condi\xc3\xa7\xc3\xb5es aqui estabelecidas, n\xc3\xa3o utilize o Aplicativo."));

    mkTitle(QString::fromUtf8("2. Descri\xc3\xa7\xc3\xa3o do Aplicativo"));
    mkBody(QString::fromUtf8(
        "O BrazaReader \xc3\xa9 um leitor de arquivos PDF desenvolvido por Brayan Henfer "
        "para dispositivos com sistema operacional Linux. O Aplicativo oferece:"));
    mkItem(QString::fromUtf8("Leitura de arquivos PDF armazenados localmente;"));
    mkItem(QString::fromUtf8("Organiza\xc3\xa7\xc3\xa3o em biblioteca pessoal e cole\xc3\xa7\xc3\xb5es;"));
    mkItem(QString::fromUtf8("Marca\xc3\xa7\xc3\xa3o de favoritos e controle de progresso;"));
    mkItem(QString::fromUtf8("Cria\xc3\xa7\xc3\xa3o e gerenciamento de anota\xc3\xa7\xc3\xb5es por p\xc3\xa1gina;"));
    mkItem(QString::fromUtf8("Ajustes de visualiza\xc3\xa7\xc3\xa3o (zoom, modo noturno, filtro \xc3\xa2mbar/s\xc3\xa9pia)."));

    mkTitle(QString::fromUtf8("3. Uso Permitido"));
    mkBody(QString::fromUtf8(
        "O Aplicativo \xc3\xa9 disponibilizado gratuitamente para uso pessoal e n\xc3\xa3o "
        "comercial. \xc3\x89 permitido ao usu\xc3\xa1rio utiliz\xc3\xa1-lo para leitura de arquivos PDF "
        "de sua pr\xc3\xb3pria autoria ou adquiridos legalmente, e compartilh\xc3\xa1-lo com "
        "terceiros sem modifica\xc3\xa7\xc3\xa3o."));

    mkTitle(QString::fromUtf8("4. Uso Proibido"));
    mkBody(QString::fromUtf8("\xc3\x89 expressamente vedado:"));
    mkItem(QString::fromUtf8(
        "Acessar, armazenar ou distribuir conte\xc3\xbado que viole direitos autorais;"));
    mkItem(QString::fromUtf8(
        "Realizar engenharia reversa ou modificar o Aplicativo sem autoriza\xc3\xa7\xc3\xa3o;"));
    mkItem(QString::fromUtf8(
        "Utilizar o Aplicativo para fins comerciais sem autoriza\xc3\xa7\xc3\xa3o pr\xc3\xa9via de Brayan Henfer."));

    mkTitle(QString::fromUtf8("5. Propriedade Intelectual"));
    mkBody(QString::fromUtf8(
        "O BrazaReader, incluindo c\xc3\xb3digo-fonte, interface, logotipo e demais elementos "
        "visuais, \xc3\xa9 de propriedade de Brayan Henfer, protegido pelas leis de propriedade "
        "intelectual aplic\xc3\xa1veis. Os arquivos PDF lidos pertencem a seus respectivos autores."));

    mkTitle(QString::fromUtf8("6. Privacidade e Dados"));
    mkSub(QString::fromUtf8("Armazenamento local"));
    mkBody(QString::fromUtf8(
        "Todos os dados gerados pelo uso do Aplicativo \xe2\x80\x94 biblioteca, anota\xc3\xa7\xc3\xb5es, "
        "favoritos, cole\xc3\xa7\xc3\xb5es e progresso \xe2\x80\x94 s\xc3\xa3o armazenados exclusivamente "
        "no dispositivo local do usu\xc3\xa1rio."));
    mkSub(QString::fromUtf8("Sem coleta de dados"));
    mkBody(QString::fromUtf8(
        "O BrazaReader n\xc3\xa3o coleta, transmite nem compartilha com terceiros nenhum dado "
        "pessoal ou de uso. N\xc3\xa3o h\xc3\xa1 telemetria, rastreamento ou conex\xc3\xa3o com a internet."));
    mkSub(QString::fromUtf8("Responsabilidade do usu\xc3\xa1rio"));
    mkBody(QString::fromUtf8(
        "O usu\xc3\xa1rio \xc3\xa9 inteiramente respons\xc3\xa1vel pela guarda e backup dos seus dados. "
        "Brayan Henfer n\xc3\xa3o se responsabiliza por perda de dados decorrente de "
        "desinstala\xc3\xa7\xc3\xa3o ou falha de hardware."));

    mkTitle(QString::fromUtf8("7. Isen\xc3\xa7\xc3\xa3o de Garantias"));
    mkBody(QString::fromUtf8(
        "O BrazaReader \xc3\xa9 fornecido \"no estado em que se encontra\" (as is), sem garantias "
        "expressas ou impl\xc3\xadcitas. Brayan Henfer n\xc3\xa3o garante compatibilidade com todos "
        "os dispositivos ou formatos de arquivo."));

    mkTitle(QString::fromUtf8("8. Limita\xc3\xa7\xc3\xa3o de Responsabilidade"));
    mkBody(QString::fromUtf8(
        "Na m\xc3\xa1xima extens\xc3\xa3o permitida por lei, Brayan Henfer n\xc3\xa3o ser\xc3\xa1 "
        "respons\xc3\xa1vel por danos diretos, indiretos ou consequenciais decorrentes "
        "do uso ou da impossibilidade de uso do Aplicativo."));

    mkTitle(QString::fromUtf8("9. Atualiza\xc3\xa7\xc3\xb5es"));
    mkBody(QString::fromUtf8(
        "Brayan Henfer reserva-se o direito de modificar o Aplicativo e estes Termos a "
        "qualquer momento. O uso continuado ap\xc3\xb3s publica\xc3\xa7\xc3\xa3o de novos Termos "
        "implica aceita\xc3\xa7\xc3\xa3o das altera\xc3\xa7\xc3\xb5es."));

    mkTitle(QString::fromUtf8("10. Legisla\xc3\xa7\xc3\xa3o Aplic\xc3\xa1vel"));
    mkBody(QString::fromUtf8(
        "Estes Termos s\xc3\xa3o regidos pela legisla\xc3\xa7\xc3\xa3o brasileira, em especial "
        "a Lei n.\xc2\xba 9.610/1998 (Direitos Autorais), a Lei n.\xc2\xba 13.709/2018 (LGPD) "
        "e o C\xc3\xb3digo de Defesa do Consumidor (Lei n.\xc2\xba 8.078/1990)."));

    // Rodapé
    QWidget* divFinal = new QWidget(content);
    divFinal->setFixedHeight(1);
    divFinal->setStyleSheet("background:#2a2a2a;margin-top:32px;margin-bottom:16px;");
    cl->addWidget(divFinal);

    QLabel* rodape = new QLabel(
        QString::fromUtf8(
            "\xc2\xa9 Brayan Henfer  \xe2\x80\x94  Todos os direitos reservados.\n"
            "BrazaReader v1.0.0"),
        content);
    rodape->setStyleSheet("color:#444;font-size:11px;");
    rodape->setAlignment(Qt::AlignCenter);
    cl->addWidget(rodape);

    cl->addStretch();
    scroll->setWidget(content);
    ml->addWidget(scroll, 1);
}
