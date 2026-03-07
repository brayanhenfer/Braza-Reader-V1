#pragma once

#include <QString>
#include <QStringList>
#include <sqlite3.h>

class FavoriteManager
{
public:
    FavoriteManager();
    ~FavoriteManager();

    bool isFavorite(const QString& bookTitle) const;
    void toggleFavorite(const QString& bookTitle);
    void addFavorite(const QString& bookTitle);
    void removeFavorite(const QString& bookTitle);
    QStringList getAllFavorites() const;
    void renameTitle(const QString& oldTitle, const QString& newTitle);

private:
    void initializeDatabase();
    void createTableIfNeeded();
    QString getDatabasePath() const;

    sqlite3* database;
};
