#pragma once

#include <QLabel>
#include <QTimer>
#include <QPoint>
#include <QRectF>
#include <QColor>
#include <QList>
#include <QPixmap>
#include <QString>

// ── TextWord ──────────────────────────────────────────────────────────────────
struct TextWord {
    QRectF  bbox;
    QString word;
};

// ── PageHighlight ─────────────────────────────────────────────────────────────
// pageRects em coordenadas de PÁGINA (pontos PDF) — convertidos em paintEvent.
struct PageHighlight {
    QList<QRectF> pageRects;
    QColor        color;
    int           annotationId = -1;
};

// ── PageWidget ────────────────────────────────────────────────────────────────
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
    void selectionFinished(const QString& text, const QList<QRectF>& pageRects);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QPoint pixmapOffset() const;
    QRectF pageToWidget(const QRectF& r) const;
    QRectF widgetToPage(const QRectF& r) const;

    QTimer pressTimer;
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
