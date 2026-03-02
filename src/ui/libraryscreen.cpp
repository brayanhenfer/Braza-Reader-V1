#include "libraryscreen.h"
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

    // ── Topbar com logo ───────────────────────────────────────────────────────
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
    logoLabel = new QLabel(topBar);
    logoLabel->setAlignment(Qt::AlignCenter);
    QPixmap logoPx(":/logo.png");
    if (!logoPx.isNull()) {
        logoLabel->setPixmap(logoPx.scaled(120, 34, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        logoLabel->setText("BrazaReader");
        logoLabel->setStyleSheet("color:white;font-size:18px;font-weight:bold;");
    }

    // Título de contexto (ex: "Favoritos")
    contextLabel = new QLabel("", topBar);
    contextLabel->setStyleSheet("color:rgba(255,255,255,0.75);font-size:13px;");
    contextLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    contextLabel->setFixedWidth(80);

    topLay->addWidget(menuButton);
    topLay->addWidget(logoLabel, 1);
    topLay->addWidget(contextLabel);
    mainLayout->addWidget(topBar);

    // ── Barra de pesquisa ─────────────────────────────────────────────────────
    // QLineEdit abre teclado virtual automaticamente em touchscreen (Qt5 com eglfs/xcb)
    searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("🔍  Buscar livro...");
    searchBar->setClearButtonEnabled(true);
    searchBar->setStyleSheet(
        "QLineEdit{background:#1a1a1a;color:white;border:none;"
        "border-bottom:2px solid #1e6432;padding:8px 12px;font-size:14px;}"
        "QLineEdit:focus{border-bottom-color:#4CAF50;}");
    connect(searchBar, &QLineEdit::textChanged, this, &LibraryScreen::onSearchTextChanged);
    mainLayout->addWidget(searchBar);

    // ── Grade de livros (sem tabs) ─────────────────────────────────────────────
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea{border:none;background:#2b2b2b;}");

    gridContainer = new QWidget();
    gridContainer->setStyleSheet("background:#2b2b2b;");
    gridLayout = new QGridLayout(gridContainer);
    gridLayout->setContentsMargins(12, 12, 12, 12);
    gridLayout->setSpacing(12);

    scrollArea->setWidget(gridContainer);
    mainLayout->addWidget(scrollArea, 1);
}

void LibraryScreen::setMenuColor(const QColor& color)
{
    topBar->setStyleSheet(QString("background-color:%1;").arg(color.name()));
}

void LibraryScreen::setWindowColor(const QColor& color)
{
    gridContainer->setStyleSheet(QString("background:%1;").arg(color.name()));
    scrollArea->setStyleSheet(
        QString("QScrollArea{border:none;background:%1;}").arg(color.name()));
}

void LibraryScreen::loadLibrary()
{
    showingFavorites = false;
    contextLabel->setText("");
    allBooksCache = libraryManager->scanLibrary();
    currentBooks  = allBooksCache;
    populateGrid(currentBooks);
}

void LibraryScreen::showFavorites()
{
    showingFavorites = true;
    contextLabel->setText("★ Favoritos");

    QStringList favs = favoriteManager->getAllFavorites();
    QStringList all  = libraryManager->scanLibrary();
    allBooksCache    = all; // mantém cache completo para pesquisa
    currentBooks.clear();
    for (const QString& p : all)
        if (favs.contains(extractTitle(p)))
            currentBooks.append(p);

    populateGrid(currentBooks);
}

void LibraryScreen::showAllBooks()
{
    loadLibrary();
}

void LibraryScreen::onSearchTextChanged(const QString& text)
{
    QStringList base = showingFavorites ? currentBooks : allBooksCache;

    if (text.trimmed().isEmpty()) {
        populateGrid(showingFavorites ? currentBooks : allBooksCache);
        return;
    }

    QStringList filtered;
    for (const QString& p : allBooksCache)
        if (extractTitle(p).contains(text.trimmed(), Qt::CaseInsensitive))
            filtered.append(p);
    populateGrid(filtered);
}

void LibraryScreen::populateGrid(const QStringList& books)
{
    clearGrid();
    if (books.isEmpty()) {
        QLabel* empty = new QLabel(
            showingFavorites ? "Nenhum favorito.\nToque em ☆ para adicionar."
                             : "Nenhum livro encontrado.", gridContainer);
        empty->setStyleSheet("color:#888;font-size:15px;");
        empty->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(empty, 0, 0, 1, 3, Qt::AlignCenter);
        return;
    }

    int row = 0, col = 0;
    const int cols = 3;
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

    // Calcula progresso sem bloquear muito tempo
    float pct = 0;
    {
        PDFRenderer tmp;
        if (tmp.openPDF(filePath)) {
            int total = tmp.getPageCount();
            if (total > 1) pct = float(lastPg) / float(total - 1) * 100.f;
            else            pct = 100.f;
            tmp.closePDF();
        }
    }

    QFrame* card = new QFrame(gridContainer);
    card->setFixedSize(210, 295);
    card->setStyleSheet("QFrame{background:#3a3a3a;border-radius:8px;}");

    QVBoxLayout* cl = new QVBoxLayout(card);
    cl->setContentsMargins(6, 6, 6, 6);
    cl->setSpacing(4);

    // Miniatura
    QLabel* thumb = new QLabel(card);
    thumb->setFixedSize(198, 195);
    thumb->setAlignment(Qt::AlignCenter);
    thumb->setStyleSheet("background:#555;border-radius:4px;color:#aaa;font-size:32px;");
    thumb->setText("📄");

    {
        PDFRenderer tmp;
        if (tmp.openPDF(filePath)) {
            QPixmap px = tmp.renderPage(0, 198, 195);
            if (!px.isNull())
                thumb->setPixmap(px.scaled(198, 195, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            tmp.closePDF();
        }
    }

    // Título
    QLabel* tl = new QLabel(title, card);
    tl->setStyleSheet("color:white;font-size:11px;");
    tl->setWordWrap(true);
    tl->setMaximumHeight(30);

    // Barra de progresso
    QWidget* progBar = new QWidget(card);
    progBar->setFixedHeight(4);
    progBar->setStyleSheet("background:#555;border-radius:2px;");
    QWidget* fill = new QWidget(progBar);
    fill->setFixedHeight(4);
    fill->setFixedWidth(qBound(0, qRound(198 * pct / 100.f), 198));
    fill->setStyleSheet("background:#4CAF50;border-radius:2px;");

    // Botões
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(4);

    QPushButton* favBtn = new QPushButton(isFav ? "★" : "☆", card);
    favBtn->setFixedSize(28, 28);
    favBtn->setStyleSheet("QPushButton{background:transparent;color:#FFD700;font-size:18px;border:none;}");

    QLabel* pctLbl = new QLabel(QString("%1%").arg(int(pct)), card);
    pctLbl->setStyleSheet("color:#4CAF50;font-size:10px;");
    pctLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QPushButton* editBtn = new QPushButton("✎", card);
    editBtn->setFixedSize(28, 28);
    editBtn->setStyleSheet("QPushButton{background:transparent;color:#aaa;font-size:16px;border:none;}");

    connect(favBtn, &QPushButton::clicked, this, [this, title, favBtn]() {
        favoriteManager->toggleFavorite(title);
        favBtn->setText(favoriteManager->isFavorite(title) ? "★" : "☆");
        if (showingFavorites) showFavorites();
    });
    connect(editBtn, &QPushButton::clicked, this, [this, filePath]() {
        onRenameBook(filePath);
    });

    btnRow->addWidget(favBtn);
    btnRow->addStretch();
    btnRow->addWidget(pctLbl);
    btnRow->addWidget(editBtn);

    cl->addWidget(thumb);
    cl->addWidget(tl);
    cl->addWidget(progBar);
    cl->addLayout(btnRow);

    // Overlay transparente para abrir livro
    QPushButton* overlay = new QPushButton(card);
    overlay->setGeometry(0, 0, 210, 250);
    overlay->setStyleSheet("background:transparent;border:none;");
    overlay->raise();
    favBtn->raise();
    editBtn->raise();
    connect(overlay, &QPushButton::clicked, this, [this, filePath]() {
        emit bookOpened(filePath);
    });

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
    // QInputDialog abre teclado virtual em touchscreen
    QString newTitle = QInputDialog::getText(this, "Renomear", "Novo nome:",
                                              QLineEdit::Normal, oldTitle, &ok);
    if (!ok || newTitle.trimmed().isEmpty() || newTitle == oldTitle) return;

    QString newPath = info.absolutePath() + "/" + newTitle.trimmed() + ".pdf";
    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "Erro", "Já existe um arquivo com esse nome.");
        return;
    }
    if (!QFile::rename(filePath, newPath)) {
        QMessageBox::warning(this, "Erro", "Não foi possível renomear.");
        return;
    }

    // Migra favoritos e progresso
    if (favoriteManager->isFavorite(oldTitle)) {
        favoriteManager->removeFavorite(oldTitle);
        favoriteManager->addFavorite(newTitle.trimmed());
    }
    int lp = progressManager->getLastPage(oldTitle);
    if (lp > 0) {
        progressManager->clearProgress(oldTitle);
        progressManager->saveProgress(newTitle.trimmed(), lp);
    }

    if (showingFavorites) showFavorites(); else loadLibrary();
}
