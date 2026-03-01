#include "termsscreen.h"
#include <QHBoxLayout>
#include <QTextEdit>

TermsScreen::TermsScreen(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

TermsScreen::~TermsScreen() = default;

void TermsScreen::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Topbar ───────────────────────────────────────────────────────────────
    topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color: #1e6432;");

    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    backButton = new QPushButton(QString::fromUtf8("←"), topBar);
    backButton->setFixedSize(40, 40);
    backButton->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 22px; border: none; }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.2); border-radius: 20px; }"
    );
    connect(backButton, &QPushButton::clicked, this, &TermsScreen::backClicked);

    titleLabel = new QLabel("Termos de Uso", topBar);
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");

    topLayout->addWidget(backButton);
    topLayout->addWidget(titleLabel, 1);
    mainLayout->addWidget(topBar);

    // ── Conteúdo ─────────────────────────────────────────────────────────────
    QTextEdit* textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet(
        "QTextEdit { background-color: #2b2b2b; color: #ddd; "
        "font-size: 14px; border: none; padding: 20px; }"
    );
    textEdit->setHtml(R"(
        <h2 style='color:#aee8b0;'>Termos de Uso — BrazaReader</h2>
        <p><b>1. Uso pessoal</b><br>
        O BrazaReader é um software de uso pessoal e educacional. É proibida a redistribuição
        do aplicativo sem autorização expressa do desenvolvedor.</p>

        <p><b>2. Conteúdo</b><br>
        O usuário é responsável pelos arquivos PDF carregados no aplicativo.
        O BrazaReader não armazena, transmite ou compartilha qualquer arquivo do usuário.</p>

        <p><b>3. Privacidade</b><br>
        O aplicativo funciona completamente offline. Nenhum dado é enviado a servidores externos.
        O progresso de leitura, favoritos e configurações são salvos apenas localmente no dispositivo.</p>

        <p><b>4. Limitação de responsabilidade</b><br>
        O software é fornecido "como está", sem garantias de qualquer tipo.
        O desenvolvedor não se responsabiliza por perda de dados ou danos causados pelo uso do aplicativo.</p>

        <p><b>5. Atualizações</b><br>
        Estes termos podem ser atualizados em versões futuras do BrazaReader.
        Recomenda-se verificar periodicamente por atualizações.</p>

        <p style='color:#888; font-size:12px;'>BrazaReader v1.0.0 — Todos os direitos reservados.</p>
    )");

    mainLayout->addWidget(textEdit, 1);
}

void TermsScreen::setMenuColor(const QColor& color)
{
    topBar->setStyleSheet(QString("background-color: %1;").arg(color.name()));
}
