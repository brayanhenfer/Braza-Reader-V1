#include "libraryscreen.h"
#include "collectionscreen.h"
#include "../storage/librarymanager.h"
#include "../storage/favoritemanager.h"
#include "../storage/progressmanager.h"
#include "../engine/pdfrenderer.h"

#include <QHBoxLayout>
#include <QFileInfo>
#include <QFrame>
#include <QInputDialog>
#include <QFile>
#include <QMessageBox>

LibraryScreen::LibraryScreen(QWidget* parent)
    : QWidget(parent)
    , showingFavorites(false)
    , libraryManager(std::make_unique<LibraryManager>())
    , favoriteManager(std::make_unique<FavoriteManager>())
    , progressManager(std::make_unique<ProgressManager>())
{
    setupUI();
}

LibraryScreen::~LibraryScreen() = default;

void LibraryScreen::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Topbar ───────────────────────────────────────────────────────────────
    topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color:#1e6432;");
    QHBoxLayout* topLay = new QHBoxLayout(topBar);
    topLay->setContentsMargins(8, 4, 8, 4);

    menuButton = new QPushButton("☰", topBar);
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuButton, &QPushButton::clicked, this, &LibraryScreen::menuClicked);

    // Logo centralizada
    titleLabel = new QLabel("BrazaReader", topBar);
    titleLabel->setStyleSheet("color:white;font-size:18px;font-weight:bold;");
    titleLabel->setAlignment(Qt::AlignCenter);

    topLay->addWidget(menuButton);
    topLay->addWidget(titleLabel, 1);
    mainLayout->addWidget(topBar);

    // ── Barra de pesquisa ─────────────────────────────────────────────────────
    // QLineEdit abre o teclado virtual do SO automaticamente no RPi com Qt5
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("🔍  Buscar livro...");
    searchBar->setClearButtonEnabled(true);
    searchBar->setStyleSheet(
        "QLineEdit{background:#1a1a1a;color:white;border:none;"
        "border-bottom:2px solid #1e6432;padding:8px 12px;font-size:14px;}"
        "QLineEdit:focus{border-bottom-color:#4CAF50;}");
    connect(searchBar, &QLineEdit::textChanged, this, &LibraryScreen::onSearchTextChanged);
    mainLayout->addWidget(searchBar);

    // ── Tabs ──────────────────────────────────────────────────────────────────
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane{border:none;background:#2b2b2b;}"
        "QTabBar::tab{background:#222;color:#aaa;padding:10px 20px;font-size:13px;}"
        "QTabBar::tab:selected{background:#1e6432;color:white;font-weight:bold;}"
        "QTabBar::tab:hover{background:#333;}");

    // Tab 1: Todos os livros
    QWidget* allTab = new QWidget();
    allTab->setStyleSheet("background:#2b2b2b;");
    QVBoxLayout* allLay = new QVBoxLayout(allTab);
    allLay->setContentsMargins(0, 0, 0, 0);

    scrollArea = new QScrollArea(allTab);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea{border:none;background:#2b2b2b;}");

    gridContainer = new QWidget();
    gridContainer->setStyleSheet("background:#2b2b2b;");
    gridLayout = new QGridLayout(gridContainer);
    gridLayout->setContentsMargins(12, 12, 12, 12);
    gridLayout->setSpacing(12);

    scrollArea->setWidget(gridContainer);
    allLay->addWidget(scrollArea);

    tabWidget->addTab(allTab, "📚 Biblioteca");

    // Tab 2: Favoritos (reutiliza o mesmo scrollArea, troca o conteúdo via showFavorites)
    QWidget* favTab = new QWidget();
    favTab->setStyleSheet("background:#2b2b2b;");
    QVBoxLayout* favLay = new QVBoxLayout(favTab);
    favLay->setContentsMargins(0, 0, 0, 0);
    QLabel* favHint = new QLabel("Use o menu lateral → Favoritos", favTab);
    favHint->setStyleSheet("color:#888;font-size:14px;");
    favHint->setAlignment(Qt::AlignCenter);
    favLay->addWidget(favHint, 1);
    tabWidget->addTab(favTab, "★ Favoritos");

    // Tab 3: Coleções
    collectionScreen = new CollectionScreen();
    connect(collectionScreen, &CollectionScreen::menuClicked, this, &LibraryScreen::menuClicked);
    connect(collectionScreen, &CollectionScreen::bookOpened, this, &LibraryScreen::bookOpened);
    tabWidget->addTab(collectionScreen, "📁 Coleções");

    mainLayout->addWidget(tabWidget, 1);

    // Muda tab "Favoritos" para mostrar a grade real
    connect(tabWidget, &QTabWidget::currentChanged, this, [this](int idx) {
        if (idx == 0) { showingFavorites = false; loadLibrary(); }
        else if (idx == 1) showFavorites();
        else if (idx == 2) collectionScreen->refresh();
    });
}

void LibraryScreen::setMenuColor(const QColor& color)
{
    topBar->setStyleSheet(QString("background-color:%1;").arg(color.name()));
    collectionScreen->setMenuColor(color);
}

void LibraryScreen::loadLibrary()
{
    showingFavorites = false;
    allBooksCache = libraryManager->scanLibrary();
    currentBooks  = allBooksCache;
    populateGrid(currentBooks);
}

void LibraryScreen::showFavorites()
{
    showingFavorites = true;
    tabWidget->setCurrentIndex(1);

    QStringList favs  = favoriteManager->getAllFavorites();
    QStringList all   = libraryManager->scanLibrary();
    currentBooks.clear();
    for (const QString& p : all)
        if (favs.contains(extractTitle(p))) currentBooks.append(p);

    populateGrid(currentBooks);
}

void LibraryScreen::showAllBooks()
{
    tabWidget->setCurrentIndex(0);
    loadLibrary();
}

void LibraryScreen::onSearchTextChanged(const QString& text)
{
    if (text.trimmed().isEmpty()) {
        currentBooks = allBooksCache;
    } else {
        currentBooks.clear();
        for (const QString& p : allBooksCache)
            if (extractTitle(p).contains(text.trimmed(), Qt::CaseInsensitive))
                currentBooks.append(p);
    }
    populateGrid(currentBooks);
}

void LibraryScreen::populateGrid(const QStringList& books)
{
    clearGrid();
    if (books.isEmpty()) {
        QLabel* empty = new QLabel("Nenhum livro encontrado.", gridContainer);
        empty->setStyleSheet("color:#888;font-size:16px;");
        empty->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(empty, 0, 0, 1, 3, Qt::AlignCenter);
        return;
    }
    int row = 0, col = 0, cols = 3;
    for (const QString& p : books) {
        addBookCard(p, row, col);
        if (++col >= cols) { col = 0; row++; }
    }
    gridLayout->setRowStretch(row + 1, 1);
}

void LibraryScreen::clearGrid()
{
    QLayoutItem* item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

void LibraryScreen::addBookCard(const QString& filePath, int row, int col)
{
    QString title  = extractTitle(filePath);
    bool    isFav  = favoriteManager->isFavorite(title);
    int     lastPg = progressManager->getLastPage(title);

    // Progresso em %
    float pct = 0;
    PDFRenderer tmpR;
    if (tmpR.openPDF(filePath)) {
        int total = tmpR.getPageCount();
        if (total > 1) pct = float(lastPg) / float(total - 1) * 100.f;
        else pct = 100.f;
        tmpR.closePDF();
    }

    QFrame* card = new QFrame(gridContainer);
    card->setFixedSize(210, 295);
    card->setStyleSheet("QFrame{background:#3a3a3a;border-radius:8px;}");

    QVBoxLayout* cl = new QVBoxLayout(card);
    cl->setContentsMargins(6, 6, 6, 6);
    cl->setSpacing(4);

    // Miniatura da capa
    QLabel* thumb = new QLabel(card);
    thumb->setFixedSize(198, 195);
    thumb->setAlignment(Qt::AlignCenter);
    thumb->setStyleSheet("background:#555;border-radius:4px;");

    PDFRenderer thumbR;
    if (thumbR.openPDF(filePath)) {
        QPixmap px = thumbR.renderPage(0, 198, 195);
        if (!px.isNull())
            thumb->setPixmap(px.scaled(198, 195, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else
            thumb->setText("📄");
        thumbR.closePDF();
    } else {
        thumb->setText("📄");
        thumb->setStyleSheet("background:#555;border-radius:4px;color:#aaa;font-size:36px;");
    }

    // Título
    QLabel* tl = new QLabel(title, card);
    tl->setStyleSheet("color:white;font-size:11px;");
    tl->setWordWrap(true);
    tl->setMaximumHeight(30);

    // Barra de progresso
    QWidget* progressBar = new QWidget(card);
    progressBar->setFixedHeight(4);
    progressBar->setStyleSheet("background:#555;border-radius:2px;");
    QWidget* progressFill = new QWidget(progressBar);
    progressFill->setFixedHeight(4);
    int fillW = qRound(198 * pct / 100.f);
    progressFill->setFixedWidth(qMax(0, qMin(198, fillW)));
    progressFill->setStyleSheet("background:#4CAF50;border-radius:2px;");

    // % lido
    QLabel* pctLabel = new QLabel(QString("%1%").arg(int(pct)), card);
    pctLabel->setStyleSheet("color:#4CAF50;font-size:10px;");
    pctLabel->setAlignment(Qt::AlignRight);

    // Botões
    QHBoxLayout* btnBar = new QHBoxLayout();
    btnBar->setSpacing(4);

    QPushButton* favBtn = new QPushButton(isFav ? "★" : "☆", card);
    favBtn->setFixedSize(28, 28);
    favBtn->setStyleSheet("QPushButton{background:transparent;color:#FFD700;font-size:18px;border:none;}");
    connect(favBtn, &QPushButton::clicked, this, [this, title, favBtn]() {
        favoriteManager->toggleFavorite(title);
        favBtn->setText(favoriteManager->isFavorite(title) ? "★" : "☆");
        if (showingFavorites) showFavorites();
    });

    QPushButton* editBtn = new QPushButton("✎", card);
    editBtn->setFixedSize(28, 28);
    editBtn->setStyleSheet("QPushButton{background:transparent;color:#aaa;font-size:16px;border:none;}");
    connect(editBtn, &QPushButton::clicked, this, [this, filePath]() { onRenameBook(filePath); });

    btnBar->addWidget(favBtn);
    btnBar->addStretch();
    btnBar->addWidget(pctLabel);
    btnBar->addWidget(editBtn);

    cl->addWidget(thumb);
    cl->addWidget(tl);
    cl->addWidget(progressBar);
    cl->addLayout(btnBar);

    // Overlay de abertura
    QPushButton* overlay = new QPushButton(card);
    overlay->setGeometry(0, 0, 210, 250);
    overlay->setStyleSheet("background:transparent;border:none;");
    overlay->raise(); favBtn->raise(); editBtn->raise();
    connect(overlay, &QPushButton::clicked, this, [this, filePath]() { onBookClicked(filePath); });

    gridLayout->addWidget(card, row, col, Qt::AlignTop);
}

QString LibraryScreen::extractTitle(const QString& filePath) const
{
    return QFileInfo(filePath).completeBaseName();
}

void LibraryScreen::onBookClicked(const QString& filePath) { emit bookOpened(filePath); }

void LibraryScreen::onFavoriteToggled(const QString& title)
{
    favoriteManager->toggleFavorite(title);
    if (showingFavorites) showFavorites();
}

void LibraryScreen::onRenameBook(const QString& filePath)
{
    QFileInfo info(filePath);
    QString oldTitle = info.completeBaseName();
    bool ok;
    QString newTitle = QInputDialog::getText(this, "Renomear PDF", "Novo nome:",
                                              QLineEdit::Normal, oldTitle, &ok);
    if (!ok || newTitle.trimmed().isEmpty() || newTitle == oldTitle) return;

    QString newPath = info.absolutePath() + "/" + newTitle.trimmed() + ".pdf";
    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "Erro", "Já existe um arquivo com esse nome."); return;
    }
    if (!QFile::rename(filePath, newPath)) {
        QMessageBox::warning(this, "Erro", "Não foi possível renomear."); return;
    }

    if (favoriteManager->isFavorite(oldTitle)) {
        favoriteManager->removeFavorite(oldTitle);
        favoriteManager->addFavorite(newTitle.trimmed());
    }
    int lp = progressManager->getLastPage(oldTitle);
    if (lp > 0) { progressManager->clearProgress(oldTitle); progressManager->saveProgress(newTitle.trimmed(), lp); }

    if (showingFavorites) showFavorites(); else loadLibrary();
}
