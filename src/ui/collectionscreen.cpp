#include "collectionscreen.h"
#include "../storage/collectionmanager.h"
#include "../storage/librarymanager.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDialog>
#include <QListWidget>

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
    QVBoxLayout* mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // ── Topbar ───────────────────────────────────────────────────────────────
    topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color:#1e6432;");
    QHBoxLayout* topLay = new QHBoxLayout(topBar);
    topLay->setContentsMargins(10, 5, 10, 5);

    menuButton = new QPushButton("☰", topBar);
    menuButton->setFixedSize(40, 40);
    menuButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuButton, &QPushButton::clicked, this, &CollectionScreen::menuClicked);

    titleLabel = new QLabel("📚 Coleções", topBar);
    titleLabel->setStyleSheet("color:white;font-size:18px;font-weight:bold;");
    topLay->addWidget(menuButton);
    topLay->addWidget(titleLabel, 1);
    mainLay->addWidget(topBar);

    // ── Corpo: dois painéis lado a lado ───────────────────────────────────────
    QWidget* body = new QWidget(this);
    body->setStyleSheet("background:#2b2b2b;");
    QHBoxLayout* bodyLay = new QHBoxLayout(body);
    bodyLay->setContentsMargins(0, 0, 0, 0);
    bodyLay->setSpacing(0);

    // ── Painel esquerdo: coleções ─────────────────────────────────────────────
    QWidget* leftPanel = new QWidget(body);
    leftPanel->setFixedWidth(220);
    leftPanel->setStyleSheet("background:#222;border-right:1px solid #444;");
    QVBoxLayout* leftLay = new QVBoxLayout(leftPanel);
    leftLay->setContentsMargins(8, 8, 8, 8);
    leftLay->setSpacing(6);

    QLabel* collLabel = new QLabel("Minhas Coleções", leftPanel);
    collLabel->setStyleSheet("color:#aaa;font-size:12px;");
    leftLay->addWidget(collLabel);

    collectionList = new QListWidget(leftPanel);
    collectionList->setStyleSheet(
        "QListWidget{background:#1a1a1a;color:white;border-radius:6px;border:none;}"
        "QListWidget::item{padding:10px;border-bottom:1px solid #333;}"
        "QListWidget::item:selected{background:#1e6432;}");
    connect(collectionList, &QListWidget::itemClicked,
            this, &CollectionScreen::onCollectionSelected);
    leftLay->addWidget(collectionList, 1);

    // Botões de gerenciamento de coleção
    newCollBtn = new QPushButton("+ Nova", leftPanel);
    newCollBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#2a8040;}");
    connect(newCollBtn, &QPushButton::clicked, this, &CollectionScreen::onNewCollection);
    leftLay->addWidget(newCollBtn);

    QHBoxLayout* collActLay = new QHBoxLayout();
    renameCollBtn = new QPushButton("✎", leftPanel);
    renameCollBtn->setFixedHeight(34);
    renameCollBtn->setStyleSheet("QPushButton{background:#444;color:white;border-radius:6px;}"
                                  "QPushButton:pressed{background:#666;}");
    connect(renameCollBtn, &QPushButton::clicked, this, &CollectionScreen::onRenameCollection);

    delCollBtn = new QPushButton("🗑", leftPanel);
    delCollBtn->setFixedHeight(34);
    delCollBtn->setStyleSheet("QPushButton{background:#8B0000;color:white;border-radius:6px;}"
                               "QPushButton:pressed{background:#A00000;}");
    connect(delCollBtn, &QPushButton::clicked, this, &CollectionScreen::onDeleteCollection);

    collActLay->addWidget(renameCollBtn);
    collActLay->addWidget(delCollBtn);
    leftLay->addLayout(collActLay);

    bodyLay->addWidget(leftPanel);

    // ── Painel direito: livros da coleção ─────────────────────────────────────
    QWidget* rightPanel = new QWidget(body);
    QVBoxLayout* rightLay = new QVBoxLayout(rightPanel);
    rightLay->setContentsMargins(12, 12, 12, 12);
    rightLay->setSpacing(8);

    collectionTitle = new QLabel("Selecione uma coleção →", rightPanel);
    collectionTitle->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
    rightLay->addWidget(collectionTitle);

    bookList = new QListWidget(rightPanel);
    bookList->setStyleSheet(
        "QListWidget{background:#1a1a1a;color:white;border-radius:6px;border:none;}"
        "QListWidget::item{padding:10px;border-bottom:1px solid #333;}"
        "QListWidget::item:selected{background:#1e6432;}"
        "QListWidget::item:hover{background:#2a2a2a;}");
    // Duplo clique → abrir livro
    connect(bookList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) emit bookOpened(path);
    });
    rightLay->addWidget(bookList, 1);

    QHBoxLayout* bookActLay = new QHBoxLayout();
    addBookBtn = new QPushButton("+ Adicionar Livro", rightPanel);
    addBookBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#2a8040;}");
    connect(addBookBtn, &QPushButton::clicked, this, &CollectionScreen::onAddBookToCollection);

    removeBookBtn = new QPushButton("− Remover", rightPanel);
    removeBookBtn->setStyleSheet(
        "QPushButton{background:#555;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#777;}");
    connect(removeBookBtn, &QPushButton::clicked, this, &CollectionScreen::onRemoveBookFromCollection);

    bookActLay->addWidget(addBookBtn);
    bookActLay->addWidget(removeBookBtn);
    rightLay->addLayout(bookActLay);

    bodyLay->addWidget(rightPanel, 1);
    mainLay->addWidget(body, 1);
}

void CollectionScreen::setMenuColor(const QColor& color)
{
    topBar->setStyleSheet(QString("background-color:%1;").arg(color.name()));
}

void CollectionScreen::refresh() { loadCollections(); }

void CollectionScreen::loadCollections()
{
    collectionList->clear();
    for (const auto& c : collManager->getAllCollections()) {
        QListWidgetItem* item = new QListWidgetItem(QString("📁 %1").arg(c.name), collectionList);
        item->setData(Qt::UserRole, c.id);
    }
}

void CollectionScreen::onCollectionSelected(QListWidgetItem* item)
{
    currentCollectionId = item->data(Qt::UserRole).toInt();
    QString name = item->text().mid(3); // remove "📁 "
    showCollectionBooks(currentCollectionId, name);
}

void CollectionScreen::showCollectionBooks(int collectionId, const QString& name)
{
    collectionTitle->setText(QString("📁 %1").arg(name));
    bookList->clear();

    QStringList titles = collManager->getBooksInCollection(collectionId);
    QStringList allBooks = libManager->scanLibrary();

    for (const QString& title : titles) {
        // Encontra o caminho completo do PDF
        QString path;
        for (const QString& p : allBooks) {
            if (QFileInfo(p).completeBaseName() == title) { path = p; break; }
        }
        QListWidgetItem* item = new QListWidgetItem(QString("📄 %1").arg(title), bookList);
        item->setData(Qt::UserRole, path);
    }
}

void CollectionScreen::onNewCollection()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Nova Coleção", "Nome da coleção:", QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        collManager->createCollection(name.trimmed());
        loadCollections();
    }
}

void CollectionScreen::onDeleteCollection()
{
    if (currentCollectionId < 0) return;
    auto btn = QMessageBox::question(this, "Excluir Coleção",
        "Excluir esta coleção? Os PDFs NÃO serão apagados.",
        QMessageBox::Yes | QMessageBox::No);
    if (btn == QMessageBox::Yes) {
        collManager->deleteCollection(currentCollectionId);
        currentCollectionId = -1;
        bookList->clear();
        collectionTitle->setText("Selecione uma coleção →");
        loadCollections();
    }
}

void CollectionScreen::onRenameCollection()
{
    if (currentCollectionId < 0) return;
    bool ok;
    QString name = QInputDialog::getText(this, "Renomear", "Novo nome:", QLineEdit::Normal, "", &ok);
    if (ok && !name.trimmed().isEmpty()) {
        collManager->renameCollection(currentCollectionId, name.trimmed());
        loadCollections();
    }
}

void CollectionScreen::onAddBookToCollection()
{
    if (currentCollectionId < 0) return;
    QStringList allBooks = libManager->scanLibrary();
    QStringList titles;
    for (const QString& p : allBooks) titles.append(QFileInfo(p).completeBaseName());

    bool ok;
    QString chosen = QInputDialog::getItem(this, "Adicionar Livro",
        "Escolha o livro:", titles, 0, false, &ok);
    if (ok && !chosen.isEmpty()) {
        collManager->addBookToCollection(currentCollectionId, chosen);
        // Atualiza painel direito
        if (auto* sel = collectionList->currentItem())
            showCollectionBooks(currentCollectionId, sel->text().mid(3));
    }
}

void CollectionScreen::onRemoveBookFromCollection()
{
    if (currentCollectionId < 0 || !bookList->currentItem()) return;
    QString title = bookList->currentItem()->text().mid(3); // remove "📄 "
    collManager->removeBookFromCollection(currentCollectionId, title);
    if (auto* sel = collectionList->currentItem())
        showCollectionBooks(currentCollectionId, sel->text().mid(3));
}
