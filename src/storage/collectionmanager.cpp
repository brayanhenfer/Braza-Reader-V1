#include "collectionmanager.h"
#include <QStandardPaths>
#include <QDir>

CollectionManager::CollectionManager() : database(nullptr) { initializeDatabase(); }
CollectionManager::~CollectionManager() { if (database) sqlite3_close(database); }

QString CollectionManager::getDatabasePath() const {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/brazareader";
    QDir().mkpath(dataPath);
    return dataPath + "/collections.db";
}

void CollectionManager::initializeDatabase() {
    if (sqlite3_open(getDatabasePath().toStdString().c_str(), &database) == SQLITE_OK) {
        sqlite3_exec(database, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
        sqlite3_exec(database, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
        createTablesIfNeeded();
    }
}

void CollectionManager::createTablesIfNeeded() {
    const char* sql = R"sql(
        CREATE TABLE IF NOT EXISTS collections (
            id   INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE NOT NULL
        );
        CREATE TABLE IF NOT EXISTS collection_books (
            id              INTEGER PRIMARY KEY AUTOINCREMENT,
            collection_name TEXT NOT NULL,
            book_title      TEXT NOT NULL,
            UNIQUE(collection_name, book_title)
        );
    )sql";
    char* err = nullptr;
    sqlite3_exec(database, sql, nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

QStringList CollectionManager::getCollections() const {
    QStringList list;
    if (!database) return list;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, "SELECT name FROM collections ORDER BY name;", -1, &stmt, nullptr) != SQLITE_OK)
        return list;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* n = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (n) list.append(QString::fromUtf8(n));
    }
    sqlite3_finalize(stmt);
    return list;
}

void CollectionManager::addCollection(const QString& name) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, "INSERT OR IGNORE INTO collections (name) VALUES (?);", -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray utf8 = name.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}

void CollectionManager::removeCollection(const QString& name) {
    if (!database) return;
    const QByteArray utf8 = name.toUtf8();

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, "DELETE FROM collection_books WHERE collection_name = ?;", -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
    }
    if (sqlite3_prepare_v2(database, "DELETE FROM collections WHERE name = ?;", -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
    }
}

void CollectionManager::addBookToCollection(const QString& collection, const QString& bookTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "INSERT OR IGNORE INTO collection_books (collection_name, book_title) VALUES (?, ?);",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray col  = collection.toUtf8();
    const QByteArray book = bookTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, col.constData(),  col.size(),  SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.constData(), book.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}

void CollectionManager::removeBookFromCollection(const QString& collection, const QString& bookTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "DELETE FROM collection_books WHERE collection_name = ? AND book_title = ?;",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray col  = collection.toUtf8();
    const QByteArray book = bookTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, col.constData(),  col.size(),  SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.constData(), book.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}

QStringList CollectionManager::getBooksInCollection(const QString& collection) const {
    QStringList list;
    if (!database) return list;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "SELECT book_title FROM collection_books WHERE collection_name = ? ORDER BY book_title;",
        -1, &stmt, nullptr) != SQLITE_OK) return list;
    const QByteArray utf8 = collection.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* t = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (t) list.append(QString::fromUtf8(t));
    }
    sqlite3_finalize(stmt);
    return list;
}

void CollectionManager::renameBookTitle(const QString& oldTitle, const QString& newTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "UPDATE collection_books SET book_title = ? WHERE book_title = ?;",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray n = newTitle.toUtf8();
    const QByteArray o = oldTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, n.constData(), n.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, o.constData(), o.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
