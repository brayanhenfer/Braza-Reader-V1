#include "collectionmanager.h"
#include <QStandardPaths>
#include <QDir>

CollectionManager::CollectionManager() : database(nullptr) { initializeDatabase(); }
CollectionManager::~CollectionManager() { if (database) sqlite3_close(database); }

QString CollectionManager::getDatabasePath() const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/brazareader";
    QDir().mkpath(path);
    return path + "/collections.db";
}

void CollectionManager::initializeDatabase()
{
    if (sqlite3_open(getDatabasePath().toStdString().c_str(), &database) == SQLITE_OK)
        createTablesIfNeeded();
}

void CollectionManager::createTablesIfNeeded()
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS collections (
            id         INTEGER PRIMARY KEY AUTOINCREMENT,
            name       TEXT UNIQUE NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        CREATE TABLE IF NOT EXISTS collection_books (
            collection_id INTEGER NOT NULL,
            book_title    TEXT NOT NULL,
            added_at      TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            PRIMARY KEY (collection_id, book_title),
            FOREIGN KEY (collection_id) REFERENCES collections(id) ON DELETE CASCADE
        );
    )";
    char* err = nullptr;
    sqlite3_exec(database, sql, nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

int CollectionManager::createCollection(const QString& name)
{
    if (!database) return -1;
    const char* sql = "INSERT OR IGNORE INTO collections (name) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, name.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(database));
}

void CollectionManager::renameCollection(int id, const QString& newName)
{
    if (!database) return;
    const char* sql = "UPDATE collections SET name=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, newName.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void CollectionManager::deleteCollection(int id)
{
    if (!database) return;
    // ON DELETE CASCADE remove os membros automaticamente
    const char* sql = "DELETE FROM collections WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

QList<Collection> CollectionManager::getAllCollections() const
{
    QList<Collection> list;
    if (!database) return list;
    const char* sql = "SELECT id, name, created_at FROM collections ORDER BY name;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Collection c;
        c.id        = sqlite3_column_int(stmt, 0);
        c.name      = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
        c.createdAt = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        list.append(c);
    }
    sqlite3_finalize(stmt);
    return list;
}

void CollectionManager::addBookToCollection(int collectionId, const QString& bookTitle)
{
    if (!database) return;
    const char* sql = "INSERT OR IGNORE INTO collection_books (collection_id, book_title) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, collectionId);
    sqlite3_bind_text(stmt, 2, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void CollectionManager::removeBookFromCollection(int collectionId, const QString& bookTitle)
{
    if (!database) return;
    const char* sql = "DELETE FROM collection_books WHERE collection_id=? AND book_title=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, collectionId);
    sqlite3_bind_text(stmt, 2, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

QStringList CollectionManager::getBooksInCollection(int collectionId) const
{
    QStringList list;
    if (!database) return list;
    const char* sql = "SELECT book_title FROM collection_books WHERE collection_id=? ORDER BY book_title;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
    sqlite3_bind_int(stmt, 1, collectionId);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        list.append(QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))));
    sqlite3_finalize(stmt);
    return list;
}

QList<int> CollectionManager::getCollectionsForBook(const QString& bookTitle) const
{
    QList<int> ids;
    if (!database) return ids;
    const char* sql = "SELECT collection_id FROM collection_books WHERE book_title=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return ids;
    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        ids.append(sqlite3_column_int(stmt, 0));
    sqlite3_finalize(stmt);
    return ids;
}

bool CollectionManager::isBookInCollection(int collectionId, const QString& bookTitle) const
{
    if (!database) return false;
    const char* sql = "SELECT COUNT(*) FROM collection_books WHERE collection_id=? AND book_title=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, collectionId);
    sqlite3_bind_text(stmt, 2, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    bool has = (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0);
    sqlite3_finalize(stmt);
    return has;
}
