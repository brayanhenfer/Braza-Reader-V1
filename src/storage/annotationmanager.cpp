#include "annotationmanager.h"
#include <QStandardPaths>
#include <QDir>

AnnotationManager::AnnotationManager()
    : database(nullptr)
{
    initializeDatabase();
}

AnnotationManager::~AnnotationManager()
{
    if (database) sqlite3_close(database);
}

QString AnnotationManager::getDatabasePath() const
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                   + "/brazareader";
    QDir().mkpath(path);
    return path + "/annotations.db";
}

void AnnotationManager::initializeDatabase()
{
    QString dbPath = getDatabasePath();
    if (sqlite3_open(dbPath.toStdString().c_str(), &database) == SQLITE_OK)
        createTableIfNeeded();
}

void AnnotationManager::createTableIfNeeded()
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS annotations (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            book_title    TEXT NOT NULL,
            page_number   INTEGER NOT NULL,
            type          INTEGER NOT NULL DEFAULT 0,
            selected_text TEXT DEFAULT '',
            note_text     TEXT DEFAULT '',
            color_r       INTEGER DEFAULT 255,
            color_g       INTEGER DEFAULT 235,
            color_b       INTEGER DEFAULT 59,
            created_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        CREATE INDEX IF NOT EXISTS idx_annotations_book
            ON annotations(book_title, page_number);
    )";
    char* err = nullptr;
    sqlite3_exec(database, sql, nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

static Annotation rowToAnnotation(sqlite3_stmt* stmt)
{
    Annotation a;
    a.id            = sqlite3_column_int(stmt, 0);
    a.bookTitle     = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
    a.pageNumber    = sqlite3_column_int(stmt, 2);
    a.type          = static_cast<Annotation::Type>(sqlite3_column_int(stmt, 3));
    a.selectedText  = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
    a.noteText      = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
    int r = sqlite3_column_int(stmt, 6);
    int g = sqlite3_column_int(stmt, 7);
    int b = sqlite3_column_int(stmt, 8);
    a.highlightColor = QColor(r, g, b);
    a.createdAt     = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)));
    return a;
}

int AnnotationManager::addHighlight(const QString& bookTitle, int page,
                                     const QString& selectedText, const QColor& color)
{
    if (!database) return -1;
    const char* sql = R"(
        INSERT INTO annotations (book_title, page_number, type, selected_text, color_r, color_g, color_b)
        VALUES (?, ?, 0, ?, ?, ?, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, page);
    sqlite3_bind_text(stmt, 3, selectedText.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, color.red());
    sqlite3_bind_int(stmt, 5, color.green());
    sqlite3_bind_int(stmt, 6, color.blue());
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(database));
}

int AnnotationManager::addNote(const QString& bookTitle, int page, const QString& noteText)
{
    if (!database) return -1;
    const char* sql = R"(
        INSERT INTO annotations (book_title, page_number, type, note_text)
        VALUES (?, ?, 1, ?);
    )";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, page);
    sqlite3_bind_text(stmt, 3, noteText.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(database));
}

int AnnotationManager::addBookmark(const QString& bookTitle, int page)
{
    if (!database) return -1;
    // Evita bookmark duplicado na mesma página
    if (hasBookmark(bookTitle, page)) return -1;
    const char* sql = R"(
        INSERT INTO annotations (book_title, page_number, type)
        VALUES (?, ?, 2);
    )";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, page);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(database));
}

QList<Annotation> AnnotationManager::getAnnotationsForPage(const QString& bookTitle, int page) const
{
    QList<Annotation> list;
    if (!database) return list;
    const char* sql = R"(
        SELECT id, book_title, page_number, type, selected_text, note_text,
               color_r, color_g, color_b, created_at
        FROM annotations WHERE book_title = ? AND page_number = ?
        ORDER BY created_at;
    )";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, page);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        list.append(rowToAnnotation(stmt));
    sqlite3_finalize(stmt);
    return list;
}

QList<Annotation> AnnotationManager::getAllAnnotations(const QString& bookTitle) const
{
    QList<Annotation> list;
    if (!database) return list;
    const char* sql = R"(
        SELECT id, book_title, page_number, type, selected_text, note_text,
               color_r, color_g, color_b, created_at
        FROM annotations WHERE book_title = ?
        ORDER BY page_number, created_at;
    )";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return list;
    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW)
        list.append(rowToAnnotation(stmt));
    sqlite3_finalize(stmt);
    return list;
}

bool AnnotationManager::hasBookmark(const QString& bookTitle, int page) const
{
    if (!database) return false;
    const char* sql = "SELECT COUNT(*) FROM annotations WHERE book_title=? AND page_number=? AND type=2;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, page);
    bool has = (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0);
    sqlite3_finalize(stmt);
    return has;
}

void AnnotationManager::updateNote(int annotationId, const QString& newText)
{
    if (!database) return;
    const char* sql = "UPDATE annotations SET note_text=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, newText.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, annotationId);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void AnnotationManager::removeAnnotation(int annotationId)
{
    if (!database) return;
    const char* sql = "DELETE FROM annotations WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_int(stmt, 1, annotationId);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void AnnotationManager::clearAllForBook(const QString& bookTitle)
{
    if (!database) return;
    const char* sql = "DELETE FROM annotations WHERE book_title=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, bookTitle.toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
