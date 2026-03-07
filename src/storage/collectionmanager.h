#pragma once
#include <QString>
#include <QStringList>
#include <sqlite3.h>
#include <memory>

class CollectionManager {
public:
    CollectionManager();
    ~CollectionManager();
    QStringList getCollections() const;
    void addCollection(const QString& name);
    void removeCollection(const QString& name);
    void addBookToCollection(const QString& collection, const QString& bookTitle);
    void removeBookFromCollection(const QString& collection, const QString& bookTitle);
    QStringList getBooksInCollection(const QString& collection) const;
    void renameBookTitle(const QString& oldTitle, const QString& newTitle);
private:
    void initializeDatabase();
    void createTablesIfNeeded();
    QString getDatabasePath() const;
    sqlite3* database = nullptr;
};
