#include "libraryscreen.h"
#include "../storage/librarymanager.h"
#include "../storage/favoritemanager.h"
#include "../storage/progressmanager.h"

#include <QHBoxLayout>
#include <QFileInfo>
#include <QFrame>

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

    // Barra superior
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

    // Área de scroll
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
        if (col >= columns) {
            col = 0;
            row++;
        }
    }
    gridLayout->setRowStretch(row + 1, 1);
}

void LibraryScreen::showFavorites()
{
    showingFavorites = true;
    titleLabel->setText("Favoritos");

    QStringList favorites = favoriteManager->getAllFavorites();
    QStringList allBooks = libraryManager->scanLibrary();

    currentBooks.clear();
    for (const QString& bookPath : allBooks) {
        QString title = extractTitle(bookPath);
        if (favorites.contains(title)) {
            currentBooks.append(bookPath);
        }
    }

    clearGrid();

    int row = 0, col = 0;
    const int columns = 3;

    for (const QString& filePath : currentBooks) {
        addBookCard(filePath, row, col);
        col++;
        if (col >= columns) {
            col = 0;
            row++;
        }
    }

    if (currentBooks.isEmpty()) {
        QLabel* emptyLabel = new QLabel(
            QString::fromUtf8("Nenhum favorito ainda.\nToque na ⭐ para adicionar."),
            gridContainer
        );
        emptyLabel->setStyleSheet("color: #888; font-size: 16px;");
        emptyLabel->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(emptyLabel, 0, 0, 1, 3, Qt::AlignCenter);
    }

    gridLayout->setRowStretch(row + 1, 1);
}

void LibraryScreen::showAllBooks()
{
    loadLibrary();
}

void LibraryScreen::clearGrid()
{
    QLayoutItem* item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void LibraryScreen::addBookCard(const QString& filePath, int row, int col)
{
    QString title = extractTitle(filePath);
    bool isFav = favoriteManager->isFavorite(title);

    QFrame* card = new QFrame(gridContainer);
    card->setFixedSize(220, 280);
    card->setStyleSheet(
        "QFrame { background-color: #3a3a3a; border-radius: 8px; }"
    );

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(8, 8, 8, 8);
    cardLayout->setSpacing(5);

    // Miniatura placeholder
    QLabel* thumbnail = new QLabel(card);
    thumbnail->setFixedSize(204, 200);
    thumbnail->setAlignment(Qt::AlignCenter);
    thumbnail->setStyleSheet(
        "background-color: #555; border-radius: 4px; color: #aaa; font-size: 40px;"
    );
    thumbnail->setText(QString::fromUtf8("📄"));

    // Título
    QLabel* titleLbl = new QLabel(title, card);
    titleLbl->setStyleSheet("color: white; font-size: 12px;");
    titleLbl->setWordWrap(true);
    titleLbl->setMaximumHeight(30);

    // Botões
    QHBoxLayout* buttonBar = new QHBoxLayout();
    buttonBar->setSpacing(5);

    QPushButton* favButton = new QPushButton(isFav ? QString::fromUtf8("★") : QString::fromUtf8("☆"), card);
    favButton->setFixedSize(30, 30);
    favButton->setStyleSheet(
        "QPushButton { background: transparent; color: #FFD700; font-size: 20px; border: none; }"
    );

    QString bookTitle = title;
    connect(favButton, &QPushButton::clicked, this, [this, bookTitle, favButton]() {
        onFavoriteToggled(bookTitle);
        bool nowFav = favoriteManager->isFavorite(bookTitle);
        favButton->setText(nowFav ? QString::fromUtf8("★") : QString::fromUtf8("☆"));
    });

    QPushButton* editButton = new QPushButton(QString::fromUtf8("✎"), card);
    editButton->setFixedSize(30, 30);
    editButton->setStyleSheet(
        "QPushButton { background: transparent; color: #aaa; font-size: 18px; border: none; }"
    );

    buttonBar->addWidget(favButton);
    buttonBar->addStretch();
    buttonBar->addWidget(editButton);

    cardLayout->addWidget(thumbnail);
    cardLayout->addWidget(titleLbl);
    cardLayout->addLayout(buttonBar);

    // Overlay para abrir livro
    QPushButton* openOverlay = new QPushButton(card);
    openOverlay->setGeometry(0, 0, 220, 240);
    openOverlay->setStyleSheet("background: transparent; border: none;");
    openOverlay->raise();
    favButton->raise();
    editButton->raise();

    QString path = filePath;
    connect(openOverlay, &QPushButton::clicked, this, [this, path]() {
        onBookClicked(path);
    });

    gridLayout->addWidget(card, row, col, Qt::AlignTop);
}

QString LibraryScreen::extractTitle(const QString& filePath) const
{
    QFileInfo info(filePath);
    return info.completeBaseName();
}

void LibraryScreen::onBookClicked(const QString& filePath)
{
    emit bookOpened(filePath);
}

void LibraryScreen::onFavoriteToggled(const QString& bookTitle)
{
    favoriteManager->toggleFavorite(bookTitle);
    if (showingFavorites) {
        showFavorites();
    }
}