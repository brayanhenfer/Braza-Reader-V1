#pragma once
#include <QString>
#include <QStringList>
#include <QList>
#include <sqlite3.h>

struct Collection {
    int     id   = -1;
    QString name;
};

class CollectionManager {
public:
    CollectionManager();
    ~CollectionManager();

    // ── Collections ───────────────────────────────────────────────────────────
    QList<Collection> getAllCollections() const;
    void              createCollection(const QString& name);
    void              deleteCollection(int id);
    void              renameCollection(int id, const QString& newName);

    // ── Books in collection ───────────────────────────────────────────────────
    QStringList getBooksInCollection(int collectionId) const;
    void        addBookToCollection(int collectionId, const QString& bookTitle);
    void        removeBookFromCollection(int collectionId, const QString& bookTitle);

    // ── Rename support ────────────────────────────────────────────────────────
    void renameBookTitle(const QString& oldTitle, const QString& newTitle);

private:
    void    initializeDatabase();
    void    createTablesIfNeeded();
    QString getDatabasePath() const;

    sqlite3* database = nullptr;
};
