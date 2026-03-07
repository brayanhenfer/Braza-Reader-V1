#pragma once
#include <QString>
#include <QColor>
#include <QList>
#include <QRectF>
#include <sqlite3.h>

struct Annotation {
    int     id           = -1;
    QString bookTitle;
    int     pageNumber   = 0;
    QString selectedText;
    QString noteText;
    QColor  highlightColor{255, 235, 59};
    QString createdAt;
    QList<QRectF> rects;
    enum Type { Highlight = 0, Note = 1, Bookmark = 2, Underline = 3 };
    Type type = Highlight;
};

class AnnotationManager {
public:
    AnnotationManager();
    ~AnnotationManager();
    int addHighlight(const QString& bookTitle, int page, const QString& selectedText,
                     const QColor& color, const QList<QRectF>& rects = {});
    int addUnderline(const QString& bookTitle, int page, const QString& selectedText,
                     const QColor& color, const QList<QRectF>& rects = {});
    int addNote(const QString& bookTitle, int page, const QString& noteText,
                const QString& selectedText = {}, const QColor& color = QColor(255,235,59),
                const QList<QRectF>& rects = {});
    int addBookmark(const QString& bookTitle, int page);
    QList<Annotation> getAnnotationsForPage(const QString& bookTitle, int page) const;
    QList<Annotation> getAllAnnotations(const QString& bookTitle) const;
    bool hasBookmark(const QString& bookTitle, int page) const;
    void updateNote(int id, const QString& newText);
    void removeAnnotation(int id);
    void renameTitle(const QString& oldTitle, const QString& newTitle);
    static QString       rectsToString(const QList<QRectF>& rects);
    static QList<QRectF> stringToRects(const QString& s);
private:
    void    initializeDatabase();
    void    createTableIfNeeded();
    QString getDatabasePath() const;
    int     insertAnnotation(const QString& bookTitle, int page, int type,
                              const QString& selectedText, const QString& noteText,
                              const QColor& color, const QList<QRectF>& rects);
    sqlite3* database = nullptr;
};
