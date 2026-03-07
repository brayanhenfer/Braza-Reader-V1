#include "annotationmanager.h"
#include <QStandardPaths>
#include <QDir>
#include <QLocale>

AnnotationManager::AnnotationManager() { initializeDatabase(); }
AnnotationManager::~AnnotationManager() { if (database) sqlite3_close(database); }

QString AnnotationManager::getDatabasePath() const {
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/brazareader";
    QDir().mkpath(dir);
    return dir + "/annotations.db";
}

void AnnotationManager::initializeDatabase() {
    if (sqlite3_open(getDatabasePath().toStdString().c_str(), &database) == SQLITE_OK) {
        // WAL mode: gravações são confirmadas imediatamente, sem lock de leitura
        sqlite3_exec(database, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
        sqlite3_exec(database, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);
        createTableIfNeeded();
    }
}

void AnnotationManager::createTableIfNeeded() {
    const char* sql = R"sql(
        CREATE TABLE IF NOT EXISTS annotations (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            book_title TEXT NOT NULL,
            page_number INTEGER NOT NULL DEFAULT 0,
            type INTEGER NOT NULL DEFAULT 0,
            selected_text TEXT NOT NULL DEFAULT '',
            note_text TEXT NOT NULL DEFAULT '',
            color_r INTEGER NOT NULL DEFAULT 255,
            color_g INTEGER NOT NULL DEFAULT 235,
            color_b INTEGER NOT NULL DEFAULT 59,
            rects TEXT NOT NULL DEFAULT '',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        CREATE INDEX IF NOT EXISTS idx_ann_book_page ON annotations(book_title, page_number);
    )sql";
    char* err = nullptr;
    sqlite3_exec(database, sql, nullptr, nullptr, &err);
    if (err) sqlite3_free(err);
}

// Serialização usando locale C (ponto decimal) e separadores distintos
QString AnnotationManager::rectsToString(const QList<QRectF>& rects) {
    QLocale c = QLocale::c();
    QStringList parts;
    for (const QRectF& r : rects)
        parts << QString("%1|%2|%3|%4")
                 .arg(c.toString(r.x(),'f',4)).arg(c.toString(r.y(),'f',4))
                 .arg(c.toString(r.width(),'f',4)).arg(c.toString(r.height(),'f',4));
    return parts.join(';');
}

QList<QRectF> AnnotationManager::stringToRects(const QString& s) {
    QList<QRectF> out;
    if (s.trimmed().isEmpty()) return out;
    QLocale c = QLocale::c();
    for (const QString& part : s.split(';')) {
        const QStringList p = part.split('|');
        if (p.size() == 4) {
            bool ok1,ok2,ok3,ok4;
            double x=c.toDouble(p[0],&ok1), y=c.toDouble(p[1],&ok2);
            double w=c.toDouble(p[2],&ok3), h=c.toDouble(p[3],&ok4);
            if (ok1&&ok2&&ok3&&ok4) out.append(QRectF(x,y,w,h));
        }
    }
    return out;
}

static Annotation rowToAnnotation(sqlite3_stmt* s) {
    Annotation a;
    a.id           = sqlite3_column_int(s,0);
    a.bookTitle    = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(s,1)));
    a.pageNumber   = sqlite3_column_int(s,2);
    a.type         = static_cast<Annotation::Type>(sqlite3_column_int(s,3));
    a.selectedText = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(s,4)));
    a.noteText     = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(s,5)));
    a.highlightColor = QColor(sqlite3_column_int(s,6),sqlite3_column_int(s,7),sqlite3_column_int(s,8));
    a.rects        = AnnotationManager::stringToRects(
        QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(s,9))));
    a.createdAt    = QString::fromUtf8(reinterpret_cast<const char*>(sqlite3_column_text(s,10)));
    return a;
}

int AnnotationManager::insertAnnotation(const QString& bookTitle, int page, int type,
                                          const QString& selectedText, const QString& noteText,
                                          const QColor& color, const QList<QRectF>& rects) {
    if (!database) return -1;
    const char* sql = "INSERT INTO annotations (book_title,page_number,type,selected_text,note_text,color_r,color_g,color_b,rects) VALUES(?,?,?,?,?,?,?,?,?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,sql,-1,&stmt,nullptr) != SQLITE_OK) return -1;
    sqlite3_bind_text(stmt,1,bookTitle.toUtf8().constData(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt,2,page);
    sqlite3_bind_int (stmt,3,type);
    sqlite3_bind_text(stmt,4,selectedText.toUtf8().constData(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt,5,noteText.toUtf8().constData(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt,6,color.red());
    sqlite3_bind_int (stmt,7,color.green());
    sqlite3_bind_int (stmt,8,color.blue());
    sqlite3_bind_text(stmt,9,rectsToString(rects).toUtf8().constData(),-1,SQLITE_TRANSIENT);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(database));
}

int AnnotationManager::addHighlight(const QString& b,int p,const QString& s,const QColor& c,const QList<QRectF>& r)
    { return insertAnnotation(b,p,0,s,"",c,r); }
int AnnotationManager::addUnderline(const QString& b,int p,const QString& s,const QColor& c,const QList<QRectF>& r)
    { return insertAnnotation(b,p,3,s,"",c,r); }
int AnnotationManager::addNote(const QString& b,int p,const QString& n,const QString& s,const QColor& c,const QList<QRectF>& r)
    { return insertAnnotation(b,p,1,s,n,c,r); }
int AnnotationManager::addBookmark(const QString& bookTitle, int page) {
    if (!database || hasBookmark(bookTitle,page)) return -1;
    return insertAnnotation(bookTitle,page,2,"","",QColor(255,200,0),{});
}

QList<Annotation> AnnotationManager::getAnnotationsForPage(const QString& bookTitle, int page) const {
    QList<Annotation> list;
    if (!database) return list;
    const char* sql = "SELECT id,book_title,page_number,type,selected_text,note_text,color_r,color_g,color_b,rects,created_at FROM annotations WHERE book_title=? AND page_number=? ORDER BY created_at;";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(database,sql,-1,&stmt,nullptr)!=SQLITE_OK) return list;
    sqlite3_bind_text(stmt,1,bookTitle.toUtf8().constData(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,2,page);
    while (sqlite3_step(stmt)==SQLITE_ROW) list.append(rowToAnnotation(stmt));
    sqlite3_finalize(stmt);
    return list;
}

QList<Annotation> AnnotationManager::getAllAnnotations(const QString& bookTitle) const {
    QList<Annotation> list;
    if (!database) return list;
    const char* sql = "SELECT id,book_title,page_number,type,selected_text,note_text,color_r,color_g,color_b,rects,created_at FROM annotations WHERE book_title=? ORDER BY page_number,created_at;";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(database,sql,-1,&stmt,nullptr)!=SQLITE_OK) return list;
    sqlite3_bind_text(stmt,1,bookTitle.toUtf8().constData(),-1,SQLITE_TRANSIENT);
    while (sqlite3_step(stmt)==SQLITE_ROW) list.append(rowToAnnotation(stmt));
    sqlite3_finalize(stmt);
    return list;
}

bool AnnotationManager::hasBookmark(const QString& bookTitle, int page) const {
    if (!database) return false;
    const char* sql = "SELECT COUNT(*) FROM annotations WHERE book_title=? AND page_number=? AND type=2;";
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(database,sql,-1,&stmt,nullptr)!=SQLITE_OK) return false;
    sqlite3_bind_text(stmt,1,bookTitle.toUtf8().constData(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,2,page);
    const bool has = sqlite3_step(stmt)==SQLITE_ROW && sqlite3_column_int(stmt,0)>0;
    sqlite3_finalize(stmt);
    return has;
}

void AnnotationManager::updateNote(int id, const QString& text) {
    if (!database) return;
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(database,"UPDATE annotations SET note_text=? WHERE id=?",-1,&stmt,nullptr)!=SQLITE_OK) return;
    sqlite3_bind_text(stmt,1,text.toUtf8().constData(),-1,SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt,2,id);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}

void AnnotationManager::removeAnnotation(int id) {
    if (!database) return;
    sqlite3_stmt* stmt=nullptr;
    if (sqlite3_prepare_v2(database,"DELETE FROM annotations WHERE id=?",-1,&stmt,nullptr)!=SQLITE_OK) return;
    sqlite3_bind_int(stmt,1,id);
    sqlite3_step(stmt); sqlite3_finalize(stmt);
}

void AnnotationManager::renameTitle(const QString& oldTitle, const QString& newTitle) {
    if (!database) return;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(database,
        "UPDATE annotations SET book_title = ? WHERE book_title = ?;",
        -1, &stmt, nullptr) != SQLITE_OK) return;
    const QByteArray n = newTitle.toUtf8();
    const QByteArray o = oldTitle.toUtf8();
    sqlite3_bind_text(stmt, 1, n.constData(), n.size(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, o.constData(), o.size(), SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}
