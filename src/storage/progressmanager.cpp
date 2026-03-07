#include "progressmanager.h"
#include <QStandardPaths>
#include <QDir>

ProgressManager::ProgressManager() : database(nullptr) { initializeDatabase(); }
ProgressManager::~ProgressManager() { if (database) sqlite3_close(database); }

QString ProgressManager::getDatabasePath() const {
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/brazareader";
    QDir().mkpath(dataPath);
    return dataPath + "/progress.db";
}

void ProgressManager::initializeDatabase() {
    if (sqlite3_open(getDatabasePath().toStdString().c_str(), &database) == SQLITE_OK)
        createTableIfNeeded();
}

void ProgressManager::createTableIfNeeded() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS book_progress (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT UNIQUE NOT NULL,
            last_page INTEGER DEFAULT 0,
            last_read TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";
    char* errMsg = nullptr;
    sqlite3_exec(database, sql, nullptr, nullptr, &errMsg);
    if (errMsg) sqlite3_free(errMsg);
}

int ProgressManager::getLastPage(const QString& bookTitle) const {
    if (!database) return 0;
    const char* sql = "SELECT last_page FROM book_progress WHERE title = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return 0;
    // SQLITE_TRANSIENT: SQLite copia a string imediatamente (evita ponteiro dangling)
    const QByteArray utf8 = bookTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    int lastPage = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        lastPage = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return lastPage;
}

void ProgressManager::saveProgress(const QString& bookTitle, int pageNumber) {
    if (!database) return;
    const char* sql = R"(
        INSERT INTO book_progress (title, last_page, last_read)
        VALUES (?, ?, CURRENT_TIMESTAMP)
        ON CONFLICT(title) DO UPDATE SET
            last_page = excluded.last_page,
            last_read = excluded.last_read;
    )";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray utf8 = bookTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, pageNumber);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void ProgressManager::clearProgress(const QString& bookTitle) {
    if (!database) return;
    const char* sql = "DELETE FROM book_progress WHERE title = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray utf8 = bookTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, utf8.constData(), utf8.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void ProgressManager::renameTitle(const QString& oldTitle, const QString& newTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "UPDATE book_progress SET title = ? WHERE title = ?;",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray n = newTitle.toUtf8();
    const QByteArray o = oldTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, n.constData(), n.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, o.constData(), o.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
