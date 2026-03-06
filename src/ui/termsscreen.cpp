#include "termsscreen.h"
#include "topbar_helper.h"
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>

TermsScreen::TermsScreen(QWidget* parent) : QWidget(parent) { setupUI(); }
TermsScreen::~TermsScreen() = default;

void TermsScreen::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Topbar
    QPushButton* menuBtn = new QPushButton(QString::fromUtf8("☰"), this);
    menuBtn->setFixedSize(40, 40);
    menuBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuBtn, &QPushButton::clicked, this, &TermsScreen::menuClicked);

    QWidget* topBar = TopBarHelper::create(this, menuBtn);
    mainLayout->addWidget(topBar);

    // Conteúdo com scroll
    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;background:#2b2b2b;}");

    QWidget* content = new QWidget();
    content->setStyleSheet("background:#2b2b2b;");
    QVBoxLayout* cl = new QVBoxLayout(content);
    cl->setContentsMargins(30, 24, 30, 24);
    cl->setSpacing(16);

    auto makeTitle = [&](const QString& t) {
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet("color:white;font-size:17px;font-weight:bold;");
        l->setWordWrap(true);
        cl->addWidget(l);
    };

    auto makeText = [&](const QString& t) {
        QLabel* l = new QLabel(t, content);
        l->setStyleSheet("color:#bbb;font-size:13px;line-height:1.6;");
        l->setWordWrap(true);
        cl->addWidget(l);
    };

    makeTitle("Termos de Uso — BrazaReader");
    makeText("Última atualização: 2025");

    makeTitle("1. Aceitação dos Termos");
    makeText("Ao utilizar o BrazaReader, você concorda com estes Termos de Uso. "
             "Se não concordar com qualquer parte, não utilize o aplicativo.");

    makeTitle("2. Uso do Aplicativo");
    makeText("O BrazaReader é um leitor de PDF pessoal. Você é responsável pelo conteúdo "
             "dos arquivos que abre. Não distribua arquivos protegidos por direitos autorais "
             "sem autorização do titular.");

    makeTitle("3. Dados e Privacidade");
    makeText("O BrazaReader armazena dados localmente em seu dispositivo: "
             "progresso de leitura, anotações, favoritos e configurações. "
             "Nenhum dado é enviado a servidores externos.");

    makeTitle("4. Propriedade Intelectual");
    makeText("O código-fonte, design e marca BrazaReader são propriedade de seus criadores. "
             "É vedada a reprodução, distribuição ou modificação sem autorização expressa.");

    makeTitle("5. Limitação de Responsabilidade");
    makeText("O aplicativo é fornecido 'como está', sem garantias de qualquer tipo. "
             "Os criadores não se responsabilizam por perda de dados ou danos decorrentes do uso.");

    makeTitle("6. Alterações");
    makeText("Estes termos podem ser atualizados a qualquer momento. "
             "O uso continuado do aplicativo após alterações implica aceitação dos novos termos.");

    cl->addStretch();
    scroll->setWidget(content);
    mainLayout->addWidget(scroll, 1);
}
