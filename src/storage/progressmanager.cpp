#include "progressmanager.h"
#include <QStandardPaths>
#include <QDir>

ProgressManager::ProgressManager()
    : database(nullptr)
{
    initializeDatabase();
}

ProgressManager::~ProgressManager()
{
    if (database) {
        sqlite3_close(database);
    }
}

QString ProgressManager::getDatabasePath() const
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/brazareader";
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(dataPath);
    }
    return dataPath + "/progress.db";
}

void ProgressManager::initializeDatabase()
{
    QString dbPath = getDatabasePath();
    int result = sqlite3_open(dbPath.toStdString().c_str(), &database);

    if (result == SQLITE_OK) {
        createTableIfNeeded();
    }
}

void ProgressManager::createTableIfNeeded()
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS book_progress (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT UNIQUE NOT NULL,
            last_page INTEGER DEFAULT 0,
            last_read TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* errMsg = nullptr;
    int result = sqlite3_exec(database, sql, nullptr, nullptr, &errMsg);

    if (result != SQLITE_OK) {
        sqlite3_free(errMsg);
    }
}

int ProgressManager::getLastPage(const QString& bookTitle) const
{
    if (!database) return 0;

    const char* sql = "SELECT last_page FROM book_progress WHERE title = ?;";
    sqlite3_stmt* stmt = nullptr;

    int result = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) return 0;

    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_STATIC);

    int lastPage = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        lastPage = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return lastPage;
}

void ProgressManager::saveProgress(const QString& bookTitle, int pageNumber)
{
    if (!database) return;

    const char* sql = R"(
        INSERT INTO book_progress (title, last_page, last_read)
        VALUES (?, ?, CURRENT_TIMESTAMP)
        ON CONFLICT(title) DO UPDATE SET
            last_page = excluded.last_page,
            last_read = excluded.last_read;
    )";

    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);

    if (result == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, pageNumber);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void ProgressManager::clearProgress(const QString& bookTitle)
{
    if (!database) return;

    const char* sql = "DELETE FROM book_progress WHERE title = ?;";
    sqlite3_stmt* stmt = nullptr;

    int result = sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}