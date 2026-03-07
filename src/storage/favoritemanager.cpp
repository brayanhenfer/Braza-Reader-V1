#include "favoritemanager.h"
#include <QStandardPaths>
#include <QDir>

FavoriteManager::FavoriteManager() : database(nullptr) { initializeDatabase(); }
FavoriteManager::~FavoriteManager() { if (database) sqlite3_close(database); }

QString FavoriteManager::getDatabasePath() const {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/brazareader";
    QDir().mkpath(dataPath);
    return dataPath + "/favorites.db";
}

void FavoriteManager::initializeDatabase() {
    if (sqlite3_open(getDatabasePath().toStdString().c_str(), &database) == SQLITE_OK)
        createTableIfNeeded();
}

void FavoriteManager::createTableIfNeeded() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS favorites (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT UNIQUE NOT NULL,
            added_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";
    char* errMsg = nullptr;
    sqlite3_exec(database, sql, nullptr, nullptr, &errMsg);
    if (errMsg) sqlite3_free(errMsg);
}

bool FavoriteManager::isFavorite(const QString& bookTitle) const {
    if (!database) return false;
    const char* sql = "SELECT COUNT(*) FROM favorites WHERE title = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    const QByteArray utf8 = bookTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    bool isFav = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        isFav = sqlite3_column_int(stmt, 0) > 0;
    sqlite3_finalize(stmt);
    return isFav;
}

void FavoriteManager::toggleFavorite(const QString& bookTitle) {
    if (isFavorite(bookTitle)) removeFavorite(bookTitle);
    else                       addFavorite(bookTitle);
}

void FavoriteManager::addFavorite(const QString& bookTitle) {
    if (!database || isFavorite(bookTitle)) return;
    const char* sql = "INSERT INTO favorites (title) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray utf8 = bookTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void FavoriteManager::removeFavorite(const QString& bookTitle) {
    if (!database) return;
    const char* sql = "DELETE FROM favorites WHERE title = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray utf8 = bookTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

QStringList FavoriteManager::getAllFavorites() const {
    QStringList favorites;
    if (!database) return favorites;
    const char* sql = "SELECT title FROM favorites ORDER BY added_date DESC;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return favorites;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (title) favorites.append(QString::fromUtf8(title));
    }
    sqlite3_finalize(stmt);
    return favorites;
}

void FavoriteManager::renameTitle(const QString& oldTitle, const QString& newTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "UPDATE favorites SET title = ? WHERE title = ?;",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray n = newTitle.toUtf8();
    const QByteArray o = oldTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, n.constData(), n.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, o.constData(), o.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
