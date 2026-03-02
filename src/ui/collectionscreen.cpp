#include "collectionscreen.h"
#include "../storage/collectionmanager.h"
#include "../storage/librarymanager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QVBoxLayout>

CollectionScreen::CollectionScreen(QWidget* parent)
    : QWidget(parent)
    , collManager(std::make_unique<CollectionManager>())
    , libManager(std::make_unique<LibraryManager>())
{
    setupUI();
    loadCollections();
}

CollectionScreen::~CollectionScreen() = default;

void CollectionScreen::setupUI()
{
    // CollectionScreen NÃO tem topbar própria.
    // Ela é embutida como widget dentro de uma aba ou tela que já tem topbar.
    QVBoxLayout* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // ── Corpo: dois painéis lado a lado ───────────────────────────────────────
    body = new QWidget(this);
    body->setStyleSheet("background:#2b2b2b;");
    QHBoxLayout* bodyLay = new QHBoxLayout(body);
    bodyLay->setContentsMargins(0, 0, 0, 0);
    bodyLay->setSpacing(0);

    // ── Painel esquerdo ───────────────────────────────────────────────────────
    QWidget* left = new QWidget(body);
    left->setFixedWidth(210);
    left->setStyleSheet("background:#1e1e1e;border-right:1px solid #444;");
    QVBoxLayout* leftLay = new QVBoxLayout(left);
    leftLay->setContentsMargins(8, 8, 8, 8);
    leftLay->setSpacing(6);

    QLabel* hdr = new QLabel("📁 Minhas Coleções", left);
    hdr->setStyleSheet("color:#ccc;font-size:12px;font-weight:bold;padding:4px 0;");
    leftLay->addWidget(hdr);

    collectionList = new QListWidget(left);
    collectionList->setStyleSheet(
        "QListWidget{background:#151515;color:white;border-radius:6px;border:none;}"
        "QListWidget::item{padding:10px 8px;border-bottom:1px solid #2a2a2a;}"
        "QListWidget::item:selected{background:#1e6432;border-radius:4px;}"
        "QListWidget::item:hover{background:#252525;}");
    connect(collectionList, &QListWidget::itemClicked,
            this, &CollectionScreen::onCollectionSelected);
    leftLay->addWidget(collectionList, 1);

    newCollBtn = new QPushButton("+ Nova Coleção", left);
    newCollBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:9px;border-radius:6px;font-size:13px;}"
        "QPushButton:pressed{background:#2a8040;}");
    connect(newCollBtn, &QPushButton::clicked, this, &CollectionScreen::onNewCollection);
    leftLay->addWidget(newCollBtn);

    QHBoxLayout* actLay = new QHBoxLayout();
    renameCollBtn = new QPushButton("✎ Renomear", left);
    renameCollBtn->setStyleSheet(
        "QPushButton{background:#444;color:white;padding:8px;border-radius:6px;font-size:12px;}"
        "QPushButton:pressed{background:#666;}");
    connect(renameCollBtn, &QPushButton::clicked, this, &CollectionScreen::onRenameCollection);

    delCollBtn = new QPushButton("🗑 Excluir", left);
    delCollBtn->setStyleSheet(
        "QPushButton{background:#7a1010;color:white;padding:8px;border-radius:6px;font-size:12px;}"
        "QPushButton:pressed{background:#a01010;}");
    connect(delCollBtn, &QPushButton::clicked, this, &CollectionScreen::onDeleteCollection);

    actLay->addWidget(renameCollBtn);
    actLay->addWidget(delCollBtn);
    leftLay->addLayout(actLay);

    bodyLay->addWidget(left);

    // ── Painel direito ────────────────────────────────────────────────────────
    QWidget* right = new QWidget(body);
    QVBoxLayout* rightLay = new QVBoxLayout(right);
    rightLay->setContentsMargins(12, 12, 12, 12);
    rightLay->setSpacing(8);

    collectionTitle = new QLabel("← Selecione uma coleção", right);
    collectionTitle->setStyleSheet("color:#ccc;font-size:15px;font-weight:bold;");
    rightLay->addWidget(collectionTitle);

    bookList = new QListWidget(right);
    bookList->setStyleSheet(
        "QListWidget{background:#1a1a1a;color:white;border-radius:6px;border:none;}"
        "QListWidget::item{padding:11px;border-bottom:1px solid #2a2a2a;}"
        "QListWidget::item:selected{background:#1e6432;}"
        "QListWidget::item:hover{background:#252525;}");
    connect(bookList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) emit bookOpened(path);
    });
    rightLay->addWidget(bookList, 1);

    QHBoxLayout* bookActLay = new QHBoxLayout();
    addBookBtn = new QPushButton("+ Adicionar Livro", right);
    addBookBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:9px;border-radius:6px;}"
        "QPushButton:pressed{background:#2a8040;}");
    connect(addBookBtn, &QPushButton::clicked, this, &CollectionScreen::onAddBookToCollection);

    removeBookBtn = new QPushButton("− Remover", right);
    removeBookBtn->setStyleSheet(
        "QPushButton{background:#444;color:white;padding:9px;border-radius:6px;}"
        "QPushButton:pressed{background:#666;}");
    connect(removeBookBtn, &QPushButton::clicked, this, &CollectionScreen::onRemoveBookFromCollection);

    bookActLay->addWidget(addBookBtn);
    bookActLay->addWidget(removeBookBtn);
    rightLay->addLayout(bookActLay);

    bodyLay->addWidget(right, 1);
    mainLay->addWidget(body, 1);
}

void CollectionScreen::setMenuColor(const QColor&) { /* sem topbar própria */ }

void CollectionScreen::setWindowColor(const QColor& color)
{
    body->setStyleSheet(QString("background:%1;").arg(color.name()));
}

void CollectionScreen::refresh() { loadCollections(); }

void CollectionScreen::loadCollections()
{
    collectionList->clear();
    for (const auto& c : collManager->getAllCollections()) {
        QListWidgetItem* item = new QListWidgetItem(QString("📁  %1").arg(c.name), collectionList);
        item->setData(Qt::UserRole, c.id);
    }
}

void CollectionScreen::onCollectionSelected(QListWidgetItem* item)
{
    currentCollectionId = item->data(Qt::UserRole).toInt();
    QString name = item->text().mid(4); // remove "📁  "
    showCollectionBooks(currentCollectionId, name);
}

void CollectionScreen::showCollectionBooks(int id, const QString& name)
{
    collectionTitle->setText(QString("📁  %1").arg(name));
    bookList->clear();

    QStringList titles = collManager->getBooksInCollection(id);
    QStringList allBooks = libManager->scanLibrary();

    for (const QString& title : titles) {
        QString path;
        for (const QString& p : allBooks)
            if (QFileInfo(p).completeBaseName() == title) { path = p; break; }
        QListWidgetItem* item = new QListWidgetItem(
            QString("📄  %1").arg(title), bookList);
        item->setData(Qt::UserRole, path);
    }
}

void CollectionScreen::onNewCollection()
{
    bool ok;
    // QInputDialog abre teclado virtual em touchscreen
    QString name = QInputDialog::getText(
        this, "Nova Coleção", "Nome da coleção:", QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        collManager->createCollection(name.trimmed());
        loadCollections();
    }
}

void CollectionScreen::onDeleteCollection()
{
    if (currentCollectionId < 0) return;
    auto btn = QMessageBox::question(this, "Excluir",
        "Excluir coleção? Os PDFs não serão removidos do disco.",
        QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::Yes) {
        collManager->deleteCollection(currentCollectionId);
        currentCollectionId = -1;
        bookList->clear();
        collectionTitle->setText("← Selecione uma coleção");
        loadCollections();
    }
}

void CollectionScreen::onRenameCollection()
{
    if (currentCollectionId < 0) return;
    bool ok;
    QString name = QInputDialog::getText(
        this, "Renomear", "Novo nome:", QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        collManager->renameCollection(currentCollectionId, name.trimmed());
        loadCollections();
    }
}

void CollectionScreen::onAddBookToCollection()
{
    if (currentCollectionId < 0) return;
    QStringList all = libManager->scanLibrary();
    QStringList titles;
    for (const QString& p : all) titles.append(QFileInfo(p).completeBaseName());
    bool ok;
    QString chosen = QInputDialog::getItem(
        this, "Adicionar Livro", "Escolha:", titles, 0, false, &ok);
    if (ok && !chosen.isEmpty()) {
        collManager->addBookToCollection(currentCollectionId, chosen);
        if (auto* sel = collectionList->currentItem())
            showCollectionBooks(currentCollectionId, sel->text().mid(4));
    }
}

void CollectionScreen::onRemoveBookFromCollection()
{
    if (currentCollectionId < 0 || !bookList->currentItem()) return;
    QString title = bookList->currentItem()->text().mid(4); // remove "📄  "
    collManager->removeBookFromCollection(currentCollectionId, title);
    if (auto* sel = collectionList->currentItem())
        showCollectionBooks(currentCollectionId, sel->text().mid(4));
}
