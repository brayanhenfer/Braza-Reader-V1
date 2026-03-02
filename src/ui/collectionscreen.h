#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QColor>
#include <memory>

class CollectionManager;
class LibraryManager;

class CollectionScreen : public QWidget
{
    Q_OBJECT

public:
    explicit CollectionScreen(QWidget* parent = nullptr);
    ~CollectionScreen();

    void refresh();
    void setMenuColor(const QColor& color);

signals:
    void menuClicked();
    void bookOpened(const QString& filePath);

private slots:
    void onNewCollection();
    void onCollectionSelected(QListWidgetItem* item);
    void onDeleteCollection();
    void onRenameCollection();
    void onAddBookToCollection();
    void onRemoveBookFromCollection();

private:
    void setupUI();
    void loadCollections();
    void showCollectionBooks(int collectionId, const QString& collectionName);

    QWidget*      topBar;
    QPushButton*  menuButton;
    QLabel*       titleLabel;

    // Painel esquerdo: lista de coleções
    QListWidget*  collectionList;
    QPushButton*  newCollBtn;
    QPushButton*  delCollBtn;
    QPushButton*  renameCollBtn;

    // Painel direito: livros da coleção selecionada
    QLabel*       collectionTitle;
    QListWidget*  bookList;
    QPushButton*  addBookBtn;
    QPushButton*  removeBookBtn;

    std::unique_ptr<CollectionManager> collManager;
    std::unique_ptr<LibraryManager>    libManager;

    int currentCollectionId = -1;
};
