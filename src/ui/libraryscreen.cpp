#include "libraryscreen.h"
#include "topbar_helper.h"
#include "../storage/librarymanager.h"
#include "../storage/favoritemanager.h"
#include "../storage/progressmanager.h"
#include "../storage/collectionmanager.h"
#include "../storage/annotationmanager.h"
#include "../engine/pdfrenderer.h"

#include <QHBoxLayout>
#include <QProgressBar>
#include <QFileInfo>
#include <QFile>
#include <QFrame>
#include <QDialog>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>

LibraryScreen::LibraryScreen(QWidget* parent)
    : QWidget(parent)
    , showingFavorites(false)
    , libraryManager(std::make_unique<LibraryManager>())
    , favoriteManager(std::make_unique<FavoriteManager>())
    , progressManager(std::make_unique<ProgressManager>())
    , collectionManager(std::make_unique<CollectionManager>())
{
    setupUI();
}

LibraryScreen::~LibraryScreen() = default;

void LibraryScreen::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* menuButton = new QPushButton(QString::fromUtf8("☰"), this);
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 24px; border: none; }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.2); border-radius: 20px; }");
    connect(menuButton, &QPushButton::clicked, this, &LibraryScreen::menuClicked);
    topBar = TopBarHelper::create(this, menuButton);
    mainLayout->addWidget(topBar);

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
    currentBooks = libraryManager->scanLibrary();
    clearGrid();
    int row = 0, col = 0;
    const int columns = 3;
    for (const QString& fp : currentBooks) {
        addBookCard(fp, row, col);
        if (++col >= columns) { col = 0; row++; }
    }
    gridLayout->setRowStretch(row + 1, 1);
}

void LibraryScreen::showFavorites()
{
    showingFavorites = true;
    QStringList favorites = favoriteManager->getAllFavorites();
    QStringList allBooks  = libraryManager->scanLibrary();
    currentBooks.clear();
    for (const QString& p : allBooks)
        if (favorites.contains(extractTitle(p))) currentBooks.append(p);

    clearGrid();
    int row = 0, col = 0;
    const int columns = 3;
    for (const QString& fp : currentBooks) {
        addBookCard(fp, row, col);
        if (++col >= columns) { col = 0; row++; }
    }
    if (currentBooks.isEmpty()) {
        auto* lbl = new QLabel(
            QString::fromUtf8("Nenhum favorito ainda.\nToque na ⭐ para adicionar."), gridContainer);
        lbl->setStyleSheet("color:#888; font-size:16px;");
        lbl->setAlignment(Qt::AlignCenter);
        gridLayout->addWidget(lbl, 0, 0, 1, 3, Qt::AlignCenter);
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


void LibraryScreen::addBookCard(const QString& filePath, int row, int col)
{
    const QString title = extractTitle(filePath);
    const bool    isFav = favoriteManager->isFavorite(title);

    // Thumbnail
    QPixmap thumbPix;
    {
        PDFRenderer thumbRenderer;
        if (thumbRenderer.openPDF(filePath)) {
            thumbPix = thumbRenderer.renderThumbnail(0, 204, 200);
            thumbRenderer.closePDF();
        }
    }

    // Progresso
    int progressPercent = 0;
    const int lastPage = progressManager->getLastPage(title);
    if (lastPage > 0) {
        PDFRenderer pr2;
        if (pr2.openPDF(filePath)) {
            const int total = pr2.getPageCount();
            pr2.closePDF();
            if (total > 1)
                progressPercent = qBound(1, int(float(lastPage) / float(total-1) * 100.f), 100);
        }
    }

    // Card
    QFrame* card = new QFrame(gridContainer);
    card->setFixedSize(220, 300);
    card->setStyleSheet("QFrame { background-color: #3a3a3a; border-radius: 8px; }");

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(8, 8, 8, 8);
    cardLayout->setSpacing(4);

    QLabel* thumbnail = new QLabel(card);
    thumbnail->setFixedSize(204, 200);
    thumbnail->setAlignment(Qt::AlignCenter);
    thumbnail->setStyleSheet("background-color:#555; border-radius:4px; color:#aaa; font-size:40px;");
    if (!thumbPix.isNull())
        thumbnail->setPixmap(thumbPix.scaled(204, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        thumbnail->setText(QString::fromUtf8("📄"));

    QLabel* titleLbl = new QLabel(title, card);
    titleLbl->setStyleSheet("color:white; font-size:11px;");
    titleLbl->setWordWrap(true);
    titleLbl->setMaximumHeight(28);

    // Barra de progresso — visual melhorado
    QWidget* progressRow = new QWidget(card);
    QHBoxLayout* progressLay = new QHBoxLayout(progressRow);
    progressLay->setContentsMargins(0,2,0,2);
    progressLay->setSpacing(6);
    QProgressBar* progressBar = new QProgressBar(progressRow);
    progressBar->setRange(0, 100);
    progressBar->setValue(progressPercent);
    progressBar->setFixedHeight(8);
    progressBar->setTextVisible(false);
    // Cor da barra baseada no progresso
    QString barColor = progressPercent == 0   ? "#555" :
                       progressPercent < 30   ? "#e67e22" :
                       progressPercent < 70   ? "#f1c40f" :
                                                "#2ecc71";
    progressBar->setStyleSheet(QString(
        "QProgressBar{background:#333;border-radius:4px;border:none;}"
        "QProgressBar::chunk{background:%1;border-radius:4px;}").arg(barColor));
    QLabel* progressLbl = new QLabel(
        progressPercent > 0 ? QString("%1%").arg(progressPercent) : QString::fromUtf8("—"),
        progressRow);
    progressLbl->setStyleSheet(QString("color:%1;font-size:10px;font-weight:bold;").arg(
        progressPercent == 0 ? "#666" : barColor));
    progressLbl->setFixedWidth(30);
    progressLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    progressLay->addWidget(progressBar, 1);
    progressLay->addWidget(progressLbl);

    // Botões
    QHBoxLayout* buttonBar = new QHBoxLayout();
    buttonBar->setSpacing(4);
    buttonBar->setContentsMargins(0,0,0,0);

    // Favorito
    QPushButton* favBtn = new QPushButton(isFav ? "★" : "☆", card);
    favBtn->setFixedSize(28, 28);
    favBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:18px;border:none;}"
        "QPushButton:pressed{color:#FFD700;}");
    const QString bkTitle = title;
    connect(favBtn, &QPushButton::clicked, this, [this, bkTitle, favBtn]() {
        onFavoriteToggled(bkTitle);
        favBtn->setText(favoriteManager->isFavorite(bkTitle) ? "★" : "☆");
    });

    // Renomear
    QPushButton* renameBtn = new QPushButton(QString::fromUtf8("✎"), card);
    renameBtn->setFixedSize(28, 28);
    renameBtn->setStyleSheet(
        "QPushButton{background:transparent;color:#aaa;font-size:16px;border:none;}"
        "QPushButton:pressed{color:white;}");
    const QString fp = filePath;
    connect(renameBtn, &QPushButton::clicked, this, [this, fp]() { onRenameBook(fp); });

    // Coleção — texto curto que renderiza em qualquer fonte
    QPushButton* collBtn = new QPushButton("Col", card);
    collBtn->setFixedSize(36, 28);
    collBtn->setToolTip("Adicionar à coleção");
    collBtn->setStyleSheet(
        "QPushButton{background:#2a5c3a;color:white;font-size:10px;font-weight:bold;"
        "border-radius:4px;border:1px solid #3a8a50;}"
        "QPushButton:pressed{background:#1e6432;}");
    connect(collBtn, &QPushButton::clicked, this, [this, bkTitle]() { onAddToCollection(bkTitle); });

    buttonBar->addWidget(favBtn);
    buttonBar->addStretch();
    buttonBar->addWidget(renameBtn);
    buttonBar->addWidget(collBtn);

    cardLayout->addWidget(thumbnail);
    cardLayout->addWidget(titleLbl);
    cardLayout->addWidget(progressRow);
    cardLayout->addLayout(buttonBar);

    // Overlay cobre apenas a área da thumbnail (sem cobrir a barra de botões)
    QPushButton* openOverlay = new QPushButton(card);
    openOverlay->setGeometry(0, 0, 220, 220);
    openOverlay->setStyleSheet("background:transparent; border:none;");
    favBtn->raise();
    renameBtn->raise();
    collBtn->raise();
    connect(openOverlay, &QPushButton::clicked, this, [this, fp]() { onBookClicked(fp); });

    gridLayout->addWidget(card, row, col, Qt::AlignTop);
}

QString LibraryScreen::extractTitle(const QString& filePath) const
{
    return QFileInfo(filePath).completeBaseName();
}

void LibraryScreen::onBookClicked(const QString& filePath) { emit bookOpened(filePath); }

void LibraryScreen::onFavoriteToggled(const QString& bookTitle)
{
    favoriteManager->toggleFavorite(bookTitle);
    if (showingFavorites) showFavorites();
}

void LibraryScreen::onRenameBook(const QString& filePath)
{
    const QString oldTitle = extractTitle(filePath);
    bool ok = false;
    const QString newName = QInputDialog::getText(
        this, "Renomear PDF", "Novo nome (sem .pdf):",
        QLineEdit::Normal, oldTitle, &ok).trimmed();
    if (!ok || newName.isEmpty() || newName == oldTitle) return;

    QFileInfo fi(filePath);
    const QString newPath = fi.absolutePath() + "/" + newName + ".pdf";
    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "Erro", "Já existe um arquivo com esse nome.");
        return;
    }
    if (!QFile::rename(filePath, newPath)) {
        QMessageBox::warning(this, "Erro", "Não foi possível renomear o arquivo.");
        return;
    }
    progressManager->renameTitle(oldTitle, newName);
    favoriteManager->renameTitle(oldTitle, newName);
    collectionManager->renameBookTitle(oldTitle, newName);
    { AnnotationManager am; am.renameTitle(oldTitle, newName); }
    if (showingFavorites) showFavorites(); else loadLibrary();
}

void LibraryScreen::onAddToCollection(const QString& bookTitle)
{
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle("Adicionar à Coleção");
    dlg->setStyleSheet("background:#1a1a1a; color:white;");
    dlg->resize(360, 380);

    auto* lay = new QVBoxLayout(dlg);
    lay->setSpacing(10);
    lay->setContentsMargins(16, 16, 16, 16);

    auto* lbl = new QLabel(
        QString("Coleção para: <b>%1</b>").arg(bookTitle), dlg);
    lbl->setStyleSheet("color:white; font-size:13px;");
    lbl->setWordWrap(true);
    lay->addWidget(lbl);

    // Lista de coleções existentes
    auto* list = new QListWidget(dlg);
    list->setStyleSheet(
        "QListWidget{background:#2b2b2b;color:white;border:1px solid #444;"
        "font-size:14px;border-radius:6px;}"
        "QListWidget::item{padding:12px;}"
        "QListWidget::item:selected{background:#1e5c28;}");
    lay->addWidget(list, 1);

    // Recarregar lista usando IDs
    auto fillList = [&]() {
        list->clear();
        const auto cols = collectionManager->getAllCollections();
        if (cols.isEmpty()) {
            auto* e = new QListWidgetItem("  Nenhuma coleção. Crie uma abaixo.");
            e->setForeground(QColor("#666"));
            list->addItem(e);
        } else {
            for (const auto& c : cols) {
                auto* item = new QListWidgetItem(
                    QString("📁  %1").arg(c.name));
                item->setData(Qt::UserRole, c.id);
                list->addItem(item);
            }
        }
    };
    fillList();

    // Linha para criar nova coleção
    auto* newRow = new QWidget(dlg);
    auto* newLay = new QHBoxLayout(newRow);
    newLay->setContentsMargins(0,0,0,0);
    newLay->setSpacing(6);
    auto* newEdit = new QLineEdit(newRow);
    newEdit->setPlaceholderText("Nome da nova coleção...");
    newEdit->setStyleSheet(
        "QLineEdit{background:#252525;color:white;border:1px solid #444;"
        "border-radius:5px;padding:6px;font-size:13px;}");
    auto* newBtn = new QPushButton("＋ Criar", newRow);
    newBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:6px 12px;"
        "border-radius:5px;font-size:13px;}"
        "QPushButton:pressed{background:#2a8040;}");
    newLay->addWidget(newEdit, 1);
    newLay->addWidget(newBtn);
    lay->addWidget(newRow);

    connect(newBtn, &QPushButton::clicked, dlg, [&, newEdit]() {
        const QString name = newEdit->text().trimmed();
        if (name.isEmpty()) return;
        collectionManager->createCollection(name);
        newEdit->clear();
        fillList();
        // Selecionar automaticamente a nova coleção
        for (int i = 0; i < list->count(); i++) {
            if (list->item(i)->text().contains(name)) {
                list->setCurrentRow(i);
                break;
            }
        }
    });

    // Botões Adicionar / Cancelar
    auto* btnRow = new QHBoxLayout();
    auto* addBtn = new QPushButton("✓ Adicionar", dlg);
    addBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:9px 20px;"
        "border-radius:6px;font-size:13px;font-weight:bold;}"
        "QPushButton:pressed{background:#2a8040;}");
    auto* cancelBtn = new QPushButton("Cancelar", dlg);
    cancelBtn->setStyleSheet(
        "QPushButton{background:#333;color:white;padding:9px 16px;"
        "border-radius:6px;font-size:13px;}"
        "QPushButton:pressed{background:#555;}");
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(addBtn);
    lay->addLayout(btnRow);

    connect(addBtn, &QPushButton::clicked, dlg, [&, dlg, bookTitle]() {
        QListWidgetItem* cur = list->currentItem();
        if (!cur) return;
        const int colId = cur->data(Qt::UserRole).toInt();
        if (colId <= 0) return;  // placeholder "Nenhuma coleção"
        collectionManager->addBookToCollection(colId, bookTitle);
        dlg->accept();
    });
    connect(cancelBtn, &QPushButton::clicked, dlg, &QDialog::reject);

    dlg->exec();
    dlg->deleteLater();
}

