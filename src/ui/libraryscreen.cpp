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
#include <QDir>
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
    topBar->setStyleSheet("background-color: #1e6432;");

    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    menuButton = new QPushButton(QString::fromUtf8("☰"), topBar);
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 24px; border: none; }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.2); border-radius: 20px; }"
    );
    connect(menuButton, &QPushButton::clicked, this, &LibraryScreen::menuClicked);

    titleLabel = new QLabel("Biblioteca", topBar);
    titleLabel->setStyleSheet("color: white; font-size: 18px; font-weight: bold;");

    topLayout->addWidget(menuButton);
    topLayout->addWidget(titleLabel, 1);
    mainLayout->addWidget(topBar);

    // ── Grid de livros ───────────────────────────────────────────────────────
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #2b2b2b; }");

    gridContainer = new QWidget();
    gridContainer->setStyleSheet("background-color: #2b2b2b;");
    gridLayout = new QGridLayout(gridContainer);
    gridLayout->setContentsMargins(15, 15, 15, 15);
    gridLayout->setSpacing(15);

    scrollArea->setWidget(gridContainer);
    mainLayout->addWidget(scrollArea, 1);
}

// ── Cor dinâmica do menu ──────────────────────────────────────────────────────

void LibraryScreen::setMenuColor(const QColor& color)
{
    topBar->setStyleSheet(QString("background-color: %1;").arg(color.name()));
}

// ── Carregar / Mostrar livros ─────────────────────────────────────────────────

void LibraryScreen::loadLibrary()
{
    showingFavorites = false;
    titleLabel->setText("Biblioteca");
    currentBooks = libraryManager->scanLibrary();
    clearGrid();

    int row = 0, col = 0;
    const int columns = 3;
    for (const QString& filePath : currentBooks) {
        addBookCard(filePath, row, col);
        col++;
        if (col >= columns) { col = 0; row++; }
    }
    gridLayout->setRowStretch(row + 1, 1);
}

// FIX: showFavorites agora filtra corretamente comparando titles via FavoriteManager
void LibraryScreen::showFavorites()
{
    showingFavorites = true;
    titleLabel->setText("Favoritos");

    QStringList favorites = favoriteManager->getAllFavorites();
    QStringList allBooks  = libraryManager->scanLibrary();

    currentBooks.clear();
    for (const QString& bookPath : allBooks) {
        if (favorites.contains(extractTitle(bookPath))) {
            currentBooks.append(bookPath);
        }
    }

    clearGrid();

    if (currentBooks.isEmpty()) {
        QLabel* emptyLabel = new QLabel(
            QString::fromUtf8("Nenhum favorito ainda.\nToque na ★ para adicionar."),
            gridContainer
        );
        emptyLabel->setStyleSheet("color: #888; font-size: 16px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(emptyLabel, 0, 0, 1, 3, Qt::AlignCenter);
        return;
    }

    int row = 0, col = 0;
    const int columns = 3;
    for (const QString& filePath : currentBooks) {
        addBookCard(filePath, row, col);
        col++;
        if (col >= columns) { col = 0; row++; }
    }
    gridLayout->setRowStretch(row + 1, 1);
}

void LibraryScreen::showAllBooks() { loadLibrary(); }

void LibraryScreen::clearGrid()
{
    QLayoutItem* item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
}

// ── Card de livro ─────────────────────────────────────────────────────────────

void LibraryScreen::addBookCard(const QString& filePath, int row, int col)
{
    QString title  = extractTitle(filePath);
    bool    isFav  = favoriteManager->isFavorite(title);

    QFrame* card = new QFrame(gridContainer);
    card->setFixedSize(220, 290);
    card->setStyleSheet("QFrame { background-color: #3a3a3a; border-radius: 8px; }");

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(8, 8, 8, 8);
    cardLayout->setSpacing(5);

    // ── Miniatura da capa (página 0 do PDF) ───────────────────────────────────
    // FIX: antes mostrava apenas emoji 📄; agora renderiza a primeira página real
    QLabel* thumbnail = new QLabel(card);
    thumbnail->setFixedSize(204, 200);
    thumbnail->setAlignment(Qt::AlignCenter);
    thumbnail->setStyleSheet("background-color: #555; border-radius: 4px;");

    // Renderiza em thread para não travar a UI (usando lambda + QPixmap fora de thread principal seria inseguro;
    // para Raspberry Pi Zero usamos renderização direta — é rápido o suficiente para miniaturas)
    PDFRenderer thumbRenderer;
    if (thumbRenderer.openPDF(filePath)) {
        QPixmap thumb = thumbRenderer.renderPage(0, 204, 200);
        if (!thumb.isNull()) {
            thumbnail->setPixmap(thumb.scaled(204, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            thumbnail->setText(QString::fromUtf8("📄"));
            thumbnail->setStyleSheet("background-color: #555; border-radius: 4px; color: #aaa; font-size: 40px;");
        }
        thumbRenderer.closePDF();
    } else {
        thumbnail->setText(QString::fromUtf8("📄"));
        thumbnail->setStyleSheet("background-color: #555; border-radius: 4px; color: #aaa; font-size: 40px;");
    }

    // ── Título ────────────────────────────────────────────────────────────────
    QLabel* titleLbl = new QLabel(title, card);
    titleLbl->setStyleSheet("color: white; font-size: 12px;");
    titleLbl->setWordWrap(true);
    titleLbl->setMaximumHeight(36);

    // ── Botões: favoritar e renomear ──────────────────────────────────────────
    QHBoxLayout* buttonBar = new QHBoxLayout();
    buttonBar->setSpacing(5);

    // FIX favoritar: o ícone atualiza corretamente e a lista de favoritos é refrescada
    QPushButton* favButton = new QPushButton(isFav ? QString::fromUtf8("★") : QString::fromUtf8("☆"), card);
    favButton->setFixedSize(30, 30);
    favButton->setStyleSheet(
        "QPushButton { background: transparent; color: #FFD700; font-size: 20px; border: none; }"
    );
    QString bookTitle = title;
    connect(favButton, &QPushButton::clicked, this, [this, bookTitle, favButton]() {
        favoriteManager->toggleFavorite(bookTitle);
        bool nowFav = favoriteManager->isFavorite(bookTitle);
        favButton->setText(nowFav ? QString::fromUtf8("★") : QString::fromUtf8("☆"));
        // FIX: se estiver na view de favoritos, atualiza a lista imediatamente
        if (showingFavorites) {
            showFavorites();
        }
    });

    // FIX renomear: abre diálogo, renomeia o arquivo e atualiza favoritos/progresso
    QPushButton* editButton = new QPushButton(QString::fromUtf8("✎"), card);
    editButton->setFixedSize(30, 30);
    editButton->setStyleSheet(
        "QPushButton { background: transparent; color: #aaa; font-size: 18px; border: none; }"
    );
    QString path = filePath;
    connect(editButton, &QPushButton::clicked, this, [this, path]() {
        onRenameBook(path);
    });

    buttonBar->addWidget(favButton);
    buttonBar->addStretch();
    buttonBar->addWidget(editButton);

    cardLayout->addWidget(thumbnail);
    cardLayout->addWidget(titleLbl);
    cardLayout->addLayout(buttonBar);

    // Overlay transparente para abrir o livro ao tocar na capa/título
    QPushButton* openOverlay = new QPushButton(card);
    openOverlay->setGeometry(0, 0, 220, 250);
    openOverlay->setStyleSheet("background: transparent; border: none;");
    openOverlay->raise();
    favButton->raise();
    editButton->raise();

    connect(openOverlay, &QPushButton::clicked, this, [this, path]() {
        onBookClicked(path);
    });

    gridLayout->addWidget(card, row, col, Qt::AlignTop);
}

QString LibraryScreen::extractTitle(const QString& filePath) const
{
    return QFileInfo(filePath).completeBaseName();
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void LibraryScreen::onBookClicked(const QString& filePath)
{
    emit bookOpened(filePath);
}

void LibraryScreen::onFavoriteToggled(const QString& bookTitle)
{
    favoriteManager->toggleFavorite(bookTitle);
    if (showingFavorites) showFavorites();
}

// FIX: renomear PDF — abre QInputDialog, renomeia arquivo físico e atualiza BD
void LibraryScreen::onRenameBook(const QString& filePath)
{
    QFileInfo info(filePath);
    QString oldTitle = info.completeBaseName();
    QString dir      = info.absolutePath();

    bool ok;
    QString newTitle = QInputDialog::getText(
        this,
        "Renomear PDF",
        "Novo nome (sem extensão):",
        QLineEdit::Normal,
        oldTitle,
        &ok
    );

    if (!ok || newTitle.trimmed().isEmpty() || newTitle == oldTitle) return;

    QString newPath = dir + "/" + newTitle.trimmed() + ".pdf";

    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "Erro", "Já existe um arquivo com esse nome.");
        return;
    }

    if (!QFile::rename(filePath, newPath)) {
        QMessageBox::warning(this, "Erro", "Não foi possível renomear o arquivo.");
        return;
    }

    // Migra favoritos e progresso para o novo nome
    if (favoriteManager->isFavorite(oldTitle)) {
        favoriteManager->removeFavorite(oldTitle);
        favoriteManager->addFavorite(newTitle.trimmed());
    }
    int lastPage = progressManager->getLastPage(oldTitle);
    if (lastPage > 0) {
        progressManager->clearProgress(oldTitle);
        progressManager->saveProgress(newTitle.trimmed(), lastPage);
    }

    // Recarrega a biblioteca com o novo nome
    if (showingFavorites) showFavorites();
    else                  loadLibrary();
}
