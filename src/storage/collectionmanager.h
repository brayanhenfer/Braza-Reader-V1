#pragma once

#include <QString>
#include <QStringList>
#include <QList>
#include <sqlite3.h>

struct Collection {
    int     id;
    QString name;
    QString createdAt;
};

class CollectionManager
{
public:
    CollectionManager();
    ~CollectionManager();

    // Coleções
    int              createCollection(const QString& name);
    void             renameCollection(int collectionId, const QString& newName);
    void             deleteCollection(int collectionId);
    QList<Collection> getAllCollections() const;

    // Membros
    void        addBookToCollection(int collectionId, const QString& bookTitle);
    void        removeBookFromCollection(int collectionId, const QString& bookTitle);
    QStringList getBooksInCollection(int collectionId) const;
    QList<int>  getCollectionsForBook(const QString& bookTitle) const;
    bool        isBookInCollection(int collectionId, const QString& bookTitle) const;

private:
    void    initializeDatabase();
    void    createTablesIfNeeded();
    QString getDatabasePath() const;

    sqlite3* database;
};
