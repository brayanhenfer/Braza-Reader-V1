#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QStringList>
#include <QColor>
#include <QPixmap>
#include <memory>

class LibraryManager;
class FavoriteManager;
class ProgressManager;

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
    void setWindowColor(const QColor& color);   // cor de fundo da grade

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

    QVBoxLayout*  mainLayout;
    QWidget*      topBar;
    QPushButton*  menuButton;
    QLabel*       logoLabel;
    QLabel*       contextLabel;   // "★ Favoritos" ou vazio
    QLineEdit*    searchBar;
    QScrollArea*  scrollArea;
    QWidget*      gridContainer;
    QGridLayout*  gridLayout;

    std::unique_ptr<LibraryManager>  libraryManager;
    std::unique_ptr<FavoriteManager> favoriteManager;
    std::unique_ptr<ProgressManager> progressManager;

    bool        showingFavorites;
    QStringList currentBooks;
    QStringList allBooksCache;
};
