#pragma once

#include <QString>
#include <QColor>
#include <QList>
#include <sqlite3.h>

struct Annotation {
    int     id;
    QString bookTitle;
    int     pageNumber;
    QString selectedText;   // texto destacado (pode ser vazio para nota pura)
    QString noteText;       // anotação escrita pelo usuário
    QColor  highlightColor; // cor do marcador
    QString createdAt;

    enum Type { Highlight, Note, Bookmark };
    Type type;
};

class AnnotationManager
{
public:
    AnnotationManager();
    ~AnnotationManager();

    // Adicionar
    int  addHighlight(const QString& bookTitle, int page,
                      const QString& selectedText, const QColor& color);
    int  addNote(const QString& bookTitle, int page, const QString& noteText);
    int  addBookmark(const QString& bookTitle, int page);

    // Consultar
    QList<Annotation> getAnnotationsForPage(const QString& bookTitle, int page) const;
    QList<Annotation> getAllAnnotations(const QString& bookTitle) const;
    bool              hasBookmark(const QString& bookTitle, int page) const;

    // Editar / Remover
    void updateNote(int annotationId, const QString& newText);
    void removeAnnotation(int annotationId);
    void clearAllForBook(const QString& bookTitle);

private:
    void    initializeDatabase();
    void    createTableIfNeeded();
    QString getDatabasePath() const;

    sqlite3* database;
};
