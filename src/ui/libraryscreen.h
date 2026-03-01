#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QStringList>
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

signals:
    void menuClicked();
    void bookOpened(const QString& filePath);

private slots:
    void onBookClicked(const QString& filePath);
    void onFavoriteToggled(const QString& bookTitle);

private:
    void setupUI();
    void clearGrid();
    void addBookCard(const QString& filePath, int row, int col);
    QString extractTitle(const QString& filePath) const;

    QVBoxLayout* mainLayout;
    QWidget* topBar;
    QPushButton* menuButton;
    QLabel* titleLabel;
    QScrollArea* scrollArea;
    QWidget* gridContainer;
    QGridLayout* gridLayout;

    std::unique_ptr<LibraryManager> libraryManager;
    std::unique_ptr<FavoriteManager> favoriteManager;
    std::unique_ptr<ProgressManager> progressManager;

    bool showingFavorites;
    QStringList currentBooks;
};