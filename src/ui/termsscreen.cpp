#include "termsscreen.h"
#include "topbar_helper.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextBrowser>

TermsScreen::TermsScreen(QWidget* parent) : QWidget(parent) { setupUI(); }
TermsScreen::~TermsScreen() = default;

void TermsScreen::setupUI()
{
    QVBoxLayout* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    backButton = new QPushButton("←");
    backButton->setFixedSize(40, 40);
    backButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:22px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    // FIX: backClicked → MainWindow::onNavigateToLibrary (não onNavigateToAbout)
    connect(backButton, &QPushButton::clicked, this, &TermsScreen::backClicked);

    topBar = TopBarHelper::create(this, backButton);
    lay->addWidget(topBar);

    QTextBrowser* text = new QTextBrowser(this);
    text->setStyleSheet(
        "QTextBrowser{background:#2b2b2b;color:#ddd;border:none;"
        "font-size:13px;padding:20px;}");
    text->setHtml(R"(
<h2 style='color:#4CAF50;'>Termos de Uso — BrazaReader</h2>
<p>Ao utilizar este software, você concorda com os termos abaixo.</p>
<h3 style='color:#aaa;'>1. Uso Pessoal</h3>
<p>O BrazaReader destina-se exclusivamente ao uso pessoal e não comercial.
   É proibida a redistribuição sem autorização expressa do desenvolvedor.</p>
<h3 style='color:#aaa;'>2. Arquivos PDF</h3>
<p>O aplicativo acessa apenas PDFs que você fornecer. Nenhum arquivo é
   transmitido para servidores externos. Todos os dados ficam localmente
   no dispositivo.</p>
<h3 style='color:#aaa;'>3. Privacidade</h3>
<p>O BrazaReader não coleta, armazena nem transmite dados pessoais.
   Progressos de leitura, favoritos e anotações são salvos apenas
   localmente no banco de dados SQLite do dispositivo.</p>
<h3 style='color:#aaa;'>4. Responsabilidade</h3>
<p>O desenvolvedor não se responsabiliza por danos causados pelo uso
   indevido do software ou por conteúdo dos PDFs lidos.</p>
<h3 style='color:#aaa;'>5. Atualizações</h3>
<p>Estes termos podem ser atualizados sem aviso prévio. Recomenda-se
   verificar esta seção periodicamente.</p>
<p style='color:#666; margin-top:30px;'>BrazaReader v1.0.0 — 2026</p>
)");
    lay->addWidget(text, 1);
}

void TermsScreen::setMenuColor(const QColor& color)
{
    TopBarHelper::setColor(topBar, color);
}
