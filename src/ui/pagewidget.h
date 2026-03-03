#pragma once

#include <QLabel>
#include <QTimer>
#include <QPoint>
#include <QRectF>
#include <QColor>
#include <QList>
#include <QPixmap>
#include <QString>

// ── TextWord ─────────────────────────────────────────────────────────────────
// Definido aqui para evitar dependência de headers externos.
// Preenchido por TextExtractor::extractWords() em readerscreen.cpp.
struct TextWord {
    QRectF  bbox;   // coordenadas em pontos PDF
    QString word;
};

// ── PageHighlight ─────────────────────────────────────────────────────────────
// Rects em coordenadas de PÁGINA (PDF pts) — convertidos para widget em paintEvent.
struct PageHighlight {
    QList<QRectF> pageRects;
    QColor        color;
    int           annotationId = -1;
};

// ── PageWidget ────────────────────────────────────────────────────────────────
// QLabel estendido com:
//   - grifos persistentes coloridos (coordenadas de página)
//   - seleção de texto por arrastar o dedo
//   - filtros âmbar e sépia
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

    // Escala: pixels_renderizados / pontos_pdf  (calculada externamente)
    void setRenderScale(float sx, float sy);

    QString        selectedText()      const;
    QList<QRectF>  selectedPageRects() const;  // em coords de página (PDF pts)

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
