#include "pagewidget.h"
#include <QPainter>
#include <QMouseEvent>

PageWidget::PageWidget(QWidget* parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);
    pressTimer.setSingleShot(true);
    pressTimer.setInterval(500);
}

void PageWidget::setPagePixmap(const QPixmap& px) { setPixmap(px); update(); }
void PageWidget::setAmberIntensity(int v)  { amberIntensity = v; update(); }
void PageWidget::setSepiaEnabled(bool e)   { sepiaEnabled   = e; update(); }
void PageWidget::setHighlights(const QList<PageHighlight>& hl) { highlights = hl; update(); }
void PageWidget::setPageWords(const QList<TextWord>& w) { pageWords = w; }
void PageWidget::setRenderScale(float sx, float sy) { scaleX = sx; scaleY = sy; }

// ── Offset do pixmap centralizado dentro do QLabel ────────────────────────────
QPoint PageWidget::pixmapOffset() const
{
    // Evita o overload depreciado pixmap() const* usando a variante Qt5 segura
    // QLabel armazena pixmap internamente; usamos width/height do pixmap via
    // uma cópia temporária escalada ao tamanho atual (já fixado pelo setFixedSize).
    // Como usamos setFixedSize(scaled.size()) em readerscreen, o label tem
    // exatamente o tamanho do pixmap — offset é sempre (0,0).
    return QPoint(0, 0);
}

// Converte bbox de página (PDF pts) → coordenadas do widget
QRectF PageWidget::pageToWidget(const QRectF& r) const
{
    const QPoint off = pixmapOffset();
    return QRectF(r.x() * scaleX + off.x(),
                  r.y() * scaleY + off.y(),
                  r.width()  * scaleX,
                  r.height() * scaleY);
}

// ── Pintura ───────────────────────────────────────────────────────────────────

void PageWidget::paintEvent(QPaintEvent* e)
{
    QLabel::paintEvent(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // 1. Highlights persistentes
    for (const PageHighlight& hl : highlights) {
        QColor c = hl.color;
        c.setAlpha(90);
        p.setBrush(c);
        p.setPen(Qt::NoPen);
        for (const QRectF& r : hl.pageRects)
            p.drawRect(pageToWidget(r));
    }

    // 2. Seleção ativa enquanto arrasta
    if (hasSel && selStart >= 0 && selEnd >= 0 && !pageWords.isEmpty()) {
        p.setBrush(QColor(100, 149, 237, 110));
        p.setPen(QPen(QColor(60, 100, 220, 160), 1));
        const int from = qMin(selStart, selEnd);
        const int to   = qMax(selStart, selEnd);
        for (int i = from; i <= to && i < pageWords.size(); i++)
            p.drawRect(pageToWidget(pageWords[i].bbox));
    }

    // 3. Filtro sépia
    if (sepiaEnabled) {
        p.setBrush(QColor(120, 80, 20, 55));
        p.setPen(Qt::NoPen);
        p.drawRect(rect());
    }

    // 4. Filtro âmbar
    if (amberIntensity > 0) {
        const int alpha = qBound(0, qRound(amberIntensity * 1.8), 200);
        p.setBrush(QColor(255, 140, 0, alpha));
        p.setPen(Qt::NoPen);
        p.drawRect(rect());
    }
}

// ── Helpers de seleção ────────────────────────────────────────────────────────

static int nearestWordIdx(const QList<TextWord>& words, const QPoint& pt,
                           float sx, float sy, QPoint off)
{
    int    best     = -1;
    double bestDist = 1e9;
    for (int i = 0; i < words.size(); i++) {
        const QRectF wr(words[i].bbox.x() * sx + off.x(),
                        words[i].bbox.y() * sy + off.y(),
                        words[i].bbox.width()  * sx,
                        words[i].bbox.height() * sy);
        const double d = qAbs(wr.center().x() - pt.x())
                       + qAbs(wr.center().y() - pt.y()) * 2.0;
        if (d < bestDist) { bestDist = d; best = i; }
    }
    return best;
}

// ── Eventos de mouse ──────────────────────────────────────────────────────────

void PageWidget::mousePressEvent(QMouseEvent* e)
{
    pressPos   = e->pos();
    isDragging = false;
    hasSel     = false;
    selStart   = selEnd = -1;
    pressTimer.start();
    update();
}

void PageWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!(e->buttons() & Qt::LeftButton)) return;
    if ((e->pos() - pressPos).manhattanLength() > 8) {
        pressTimer.stop();
        isDragging = true;
    }
    if (isDragging && !pageWords.isEmpty()) {
        dragPos = e->pos();
        const QPoint off = pixmapOffset();
        selStart = nearestWordIdx(pageWords, pressPos, scaleX, scaleY, off);
        selEnd   = nearestWordIdx(pageWords, dragPos,  scaleX, scaleY, off);
        hasSel   = (selStart >= 0 && selEnd >= 0);
        update();
    }
}

void PageWidget::mouseReleaseEvent(QMouseEvent*)
{
    pressTimer.stop();

    if (isDragging && hasSel && !pageWords.isEmpty()) {
        const int from = qMin(selStart, selEnd);
        const int to   = qMax(selStart, selEnd);

        QString       text;
        QList<QRectF> pageRects;
        for (int i = from; i <= to && i < pageWords.size(); i++) {
            if (!text.isEmpty()) text += ' ';
            text += pageWords[i].word;
            pageRects.append(pageWords[i].bbox);
        }

        hasSel = false; selStart = selEnd = -1;
        update();

        if (!text.trimmed().isEmpty())
            emit selectionFinished(text.trimmed(), pageRects);

    } else if (!isDragging) {
        hasSel = false; selStart = selEnd = -1;
        update();
    }
    isDragging = false;
}

QString PageWidget::selectedText() const
{
    if (!hasSel || selStart < 0) return {};
    const int from = qMin(selStart, selEnd);
    const int to   = qMax(selStart, selEnd);
    QString t;
    for (int i = from; i <= to && i < pageWords.size(); i++) {
        if (!t.isEmpty()) t += ' ';
        t += pageWords[i].word;
    }
    return t;
}

QList<QRectF> PageWidget::selectedPageRects() const
{
    QList<QRectF> rects;
    if (!hasSel || selStart < 0) return rects;
    const int from = qMin(selStart, selEnd);
    const int to   = qMax(selStart, selEnd);
    for (int i = from; i <= to && i < pageWords.size(); i++)
        rects.append(pageWords[i].bbox);
    return rects;
}
