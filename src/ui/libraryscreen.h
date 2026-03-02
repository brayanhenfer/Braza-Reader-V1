#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QStringList>
#include <QColor>
#include <memory>

class LibraryManager;
class FavoriteManager;
class ProgressManager;
class CollectionScreen;

class LibraryScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LibraryScreen(QWidget* parent = nullptr);
    ~LibraryScreen();

    void loadLibrary();
    void showFavorites();
    void showAllBooks();
    void setMenuColor(const QColor& color);

signals:
    void menuClicked();
    void bookOpened(const QString& filePath);

private slots:
    void onBookClicked(const QString& filePath);
    void onFavoriteToggled(const QString& bookTitle);
    void onRenameBook(const QString& filePath);
    void onSearchTextChanged(const QString& text);

private:
    void setupUI();
    void clearGrid();
    void addBookCard(const QString& filePath, int row, int col);
    void populateGrid(const QStringList& books);
    QString extractTitle(const QString& filePath) const;

    QVBoxLayout*   mainLayout;
    QWidget*       topBar;
    QPushButton*   menuButton;
    QLabel*        titleLabel;

    // Barra de pesquisa com teclado virtual (QLineEdit abre teclado do SO)
    QLineEdit*     searchBar;

    // Tabs: Todos | Favoritos | Coleções
    QTabWidget*    tabWidget;

    // Tab Todos/Favoritos compartilha o mesmo grid
    QScrollArea*   scrollArea;
    QWidget*       gridContainer;
    QGridLayout*   gridLayout;

    // Tab Coleções
    CollectionScreen* collectionScreen;

    std::unique_ptr<LibraryManager>  libraryManager;
    std::unique_ptr<FavoriteManager> favoriteManager;
    std::unique_ptr<ProgressManager> progressManager;

    bool        showingFavorites;
    QStringList currentBooks;
    QStringList allBooksCache;
};
