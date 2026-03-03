#pragma once

#include <QString>
#include <QColor>
#include <QList>
#include <QRectF>
#include <sqlite3.h>

// ── Annotation ────────────────────────────────────────────────────────────────

struct Annotation {
    int     id           = -1;
    QString bookTitle;
    int     pageNumber   = 0;
    QString selectedText;
    QString noteText;
    QColor  highlightColor{255, 235, 59};
    QString createdAt;
    QList<QRectF> rects;        // coords de página (PDF pts), serializado no banco

    enum Type { Highlight = 0, Note = 1, Bookmark = 2 };
    Type type = Highlight;
};

// ── AnnotationManager ─────────────────────────────────────────────────────────

class AnnotationManager
{
public:
    AnnotationManager();
    ~AnnotationManager();

    // Adiciona grifo simples (sem nota)
    int addHighlight(const QString& bookTitle,
                     int            page,
                     const QString& selectedText,
                     const QColor&  color,
                     const QList<QRectF>& rects = {});

    // Adiciona grifo com nota
    int addNote(const QString& bookTitle,
                int            page,
                const QString& noteText,
                const QString& selectedText  = {},
                const QColor&  color         = QColor(255, 235, 59),
                const QList<QRectF>& rects   = {});

    // Adiciona marcador de página
    int addBookmark(const QString& bookTitle, int page);

    // Consultas
    QList<Annotation> getAnnotationsForPage(const QString& bookTitle, int page) const;
    QList<Annotation> getAllAnnotations(const QString& bookTitle) const;
    bool              hasBookmark(const QString& bookTitle, int page) const;

    // Edição / Exclusão
    void updateNote(int id, const QString& newText);
    void removeAnnotation(int id);

    // Serialização pública (útil para testes)
    static QString       rectsToString(const QList<QRectF>& rects);
    static QList<QRectF> stringToRects(const QString& s);

private:
    void    initializeDatabase();
    void    createTableIfNeeded();
    QString getDatabasePath() const;

    sqlite3* database = nullptr;
};
