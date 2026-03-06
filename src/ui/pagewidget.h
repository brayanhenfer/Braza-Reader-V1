#pragma once

#include <QLabel>
#include <QTimer>
#include <QPoint>
#include <QRectF>
#include <QColor>
#include <QList>
#include <QPixmap>
#include <QString>

struct TextWord {
    QRectF  bbox;
    QString word;
};

struct PageHighlight {
    QList<QRectF> pageRects;   // coords de PÁGINA (PDF pts)
    QColor        color;
    int           annotationId = -1;
};

class PageWidget : public QLabel
{
    Q_OBJECT
public:
    explicit PageWidget(QWidget* parent = nullptr);

    void setPagePixmap(const QPixmap& px);
    void setAmberIntensity(int v);
    void setSepiaEnabled(bool e);
    void setHighlights(const QList<PageHighlight>& hl);
    void setPageWords(const QList<TextWord>& words);
    void setRenderScale(float sx, float sy);

    QString        selectedText()      const;
    QList<QRectF>  selectedPageRects() const;

signals:
    // Emitido quando o usuário termina uma seleção por arrastar
    void selectionFinished(const QString& text, const QList<QRectF>& pageRects);
    // Emitido em tap simples (sem arrastar) — usado para toggle da topbar
    void pageTapped();

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QRectF pageToWidget(const QRectF& r) const;
    QRectF widgetToPage(const QRectF& r) const;

    QPoint pressPos;
    QPoint dragPos;
    bool   isDragging    = false;
    bool   hasSel        = false;

    int    amberIntensity = 0;
    bool   sepiaEnabled   = false;

    float  scaleX = 1.f;
    float  scaleY = 1.f;

    QList<TextWord>      pageWords;
    int selStart = -1, selEnd = -1;

    QList<PageHighlight> highlights;
};
