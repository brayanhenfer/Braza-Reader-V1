#include "collectionmanager.h"
#include <QStandardPaths>
#include <QDir>

CollectionManager::CollectionManager() : database(nullptr) { initializeDatabase(); }
CollectionManager::~CollectionManager() { if (database) sqlite3_close(database); }

QString CollectionManager::getDatabasePath() const {
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                   + "/brazareader";
    QDir().mkpath(path);
    return path + "/collections.db";
}

void CollectionManager::initializeDatabase() {
    if (sqlite3_open(getDatabasePath().toStdString().c_str(), &database) == SQLITE_OK) {
        sqlite3_exec(database, "PRAGMA journal_mode=WAL;",   nullptr, nullptr, nullptr);
        sqlite3_exec(database, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
        createTablesIfNeeded();
    }
}

void CollectionManager::createTablesIfNeeded() {
    char* err = nullptr;
    sqlite3_exec(database,
        "CREATE TABLE IF NOT EXISTS collections ("
        "  id   INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT UNIQUE NOT NULL"
        ");",
        nullptr, nullptr, &err);
    if (err) sqlite3_free(err);

    sqlite3_exec(database,
        "CREATE TABLE IF NOT EXISTS collection_books ("
        "  id              INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  collection_id   INTEGER NOT NULL,"
        "  book_title      TEXT NOT NULL,"
        "  UNIQUE(collection_id, book_title)"
        ");",
        nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

// ── Collections ───────────────────────────────────────────────────────────────

QList<Collection> CollectionManager::getAllCollections() const {
    QList<Collection> list;
    if (!database) return list;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "SELECT id, name FROM collections ORDER BY name;",
        -1, &stmt, nullptr) != SQLITE_OK) return list;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Collection c;
        c.id   = sqlite3_column_int(stmt, 0);
        const char* n = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        c.name = n ? QString::fromUtf8(n) : QString();
        list.append(c);
    }
    sqlite3_finalize(stmt);
    return list;
}

void CollectionManager::createCollection(const QString& name) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "INSERT OR IGNORE INTO collections (name) VALUES (?);",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray utf8 = name.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void CollectionManager::deleteCollection(int id) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    // Remove livros primeiro
    if (sqlite3_prepare_v2(database,
        "DELETE FROM collection_books WHERE collection_id=?;",
        -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
    }
    // Remove coleção
    if (sqlite3_prepare_v2(database,
        "DELETE FROM collections WHERE id=?;",
        -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt); sqlite3_finalize(stmt);
    }
}

void CollectionManager::renameCollection(int id, const QString& newName) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "UPDATE collections SET name=? WHERE id=?;",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray utf8 = newName.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, id);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}

// ── Books in collection ───────────────────────────────────────────────────────

QStringList CollectionManager::getBooksInCollection(int collectionId) const {
    QStringList list;
    if (!database) return list;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "SELECT book_title FROM collection_books WHERE collection_id=? ORDER BY book_title;",
        -1, &stmt, nullptr) != SQLITE_OK) return list;
    sqlite3_bind_int(stmt, 1, collectionId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* t = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (t) list.append(QString::fromUtf8(t));
    }
    sqlite3_finalize(stmt);
    return list;
}

void CollectionManager::addBookToCollection(int collectionId, const QString& bookTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "INSERT OR IGNORE INTO collection_books (collection_id, book_title) VALUES (?, ?);",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray book = bookTitle.toUtf8();
    sqlite3_bind_int (stmt, 1, collectionId);
    sqlite3_bind_text(stmt, 2, book.constData(), book.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}

void CollectionManager::removeBookFromCollection(int collectionId, const QString& bookTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "DELETE FROM collection_books WHERE collection_id=? AND book_title=?;",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray book = bookTitle.toUtf8();
    sqlite3_bind_int (stmt, 1, collectionId);
    sqlite3_bind_text(stmt, 2, book.constData(), book.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}

void CollectionManager::renameBookTitle(const QString& oldTitle, const QString& newTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "UPDATE collection_books SET book_title=? WHERE book_title=?;",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray n = newTitle.toUtf8();
    const QByteArray o = oldTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, n.constData(), n.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, o.constData(), o.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}
