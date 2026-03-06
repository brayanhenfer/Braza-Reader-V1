#include "pagewidget.h"
#include <QPainter>
#include <QMouseEvent>

PageWidget::PageWidget(QWidget* parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);
}

void PageWidget::setPagePixmap(const QPixmap& px)  { setPixmap(px); update(); }
void PageWidget::setAmberIntensity(int v)           { amberIntensity = v; update(); }
void PageWidget::setSepiaEnabled(bool e)            { sepiaEnabled = e; update(); }
void PageWidget::setHighlights(const QList<PageHighlight>& hl) { highlights = hl; update(); }
void PageWidget::setPageWords(const QList<TextWord>& w) { pageWords = w; }
void PageWidget::setRenderScale(float sx, float sy) { scaleX = sx; scaleY = sy; }

QRectF PageWidget::pageToWidget(const QRectF& r) const
{
    return QRectF(r.x() * scaleX, r.y() * scaleY,
                  r.width() * scaleX, r.height() * scaleY);
}

QRectF PageWidget::widgetToPage(const QRectF& r) const
{
    if (scaleX <= 0 || scaleY <= 0) return r;
    return QRectF(r.x() / scaleX, r.y() / scaleY,
                  r.width() / scaleX, r.height() / scaleY);
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
        c.setAlpha(110);
        p.setBrush(c);
        p.setPen(Qt::NoPen);
        for (const QRectF& r : hl.pageRects)
            p.drawRect(pageToWidget(r));
    }

    // 2a. Seleção por palavras (extração de texto disponível)
    if (hasSel && selStart >= 0 && selEnd >= 0 && !pageWords.isEmpty()) {
        p.setBrush(QColor(80, 130, 220, 100));
        p.setPen(QPen(QColor(60, 100, 200, 180), 1));
        const int from = qMin(selStart, selEnd);
        const int to   = qMax(selStart, selEnd);
        for (int i = from; i <= to && i < pageWords.size(); i++)
            p.drawRect(pageToWidget(pageWords[i].bbox));
    }

    // 2b. Rubber-band (sem extração de texto)
    if (hasSel && pageWords.isEmpty() && isDragging) {
        const QRect rr = QRect(pressPos, dragPos).normalized();
        p.setBrush(QColor(80, 130, 220, 60));
        p.setPen(QPen(QColor(60, 100, 200, 220), 2, Qt::DashLine));
        p.drawRect(rr);
    }

    // 3. Filtro sépia
    if (sepiaEnabled) {
        p.setBrush(QColor(120, 80, 20, 55));
        p.setPen(Qt::NoPen);
        p.drawRect(rect());
    }

    // 4. Filtro âmbar
    if (amberIntensity > 0) {
        p.setBrush(QColor(255, 140, 0, qBound(0, qRound(amberIntensity * 1.8), 200)));
        p.setPen(Qt::NoPen);
        p.drawRect(rect());
    }
}

// ── Helpers de seleção por palavras ──────────────────────────────────────────

static int nearestWordIdx(const QList<TextWord>& words, const QPoint& pt, float sx, float sy)
{
    int best = -1; double bestD = 1e9;
    for (int i = 0; i < words.size(); i++) {
        const QRectF wr(words[i].bbox.x() * sx, words[i].bbox.y() * sy,
                        words[i].bbox.width() * sx, words[i].bbox.height() * sy);
        double d = qAbs(wr.center().x() - pt.x())
                 + qAbs(wr.center().y() - pt.y()) * 2.0;
        if (d < bestD) { bestD = d; best = i; }
    }
    return best;
}

// ── Eventos de mouse ──────────────────────────────────────────────────────────

void PageWidget::mousePressEvent(QMouseEvent* e)
{
    pressPos   = e->pos();
    dragPos    = e->pos();
    isDragging = false;
    hasSel     = false;
    selStart   = selEnd = -1;
    update();
}

void PageWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!(e->buttons() & Qt::LeftButton)) return;
    if ((e->pos() - pressPos).manhattanLength() > 10) isDragging = true;

    if (isDragging) {
        dragPos = e->pos();
        if (!pageWords.isEmpty()) {
            selStart = nearestWordIdx(pageWords, pressPos, scaleX, scaleY);
            selEnd   = nearestWordIdx(pageWords, dragPos,  scaleX, scaleY);
        }
        hasSel = true;
        update();
    }
}

void PageWidget::mouseReleaseEvent(QMouseEvent*)
{
    const bool wasDragging = isDragging;
    isDragging = false;

    if (!wasDragging) {
        // ── Tap simples: toggle topbar/bottombar ──────────────────────────
        hasSel = false; selStart = selEnd = -1; update();
        emit pageTapped();
        return;
    }

    if (hasSel) {
        if (!pageWords.isEmpty()) {
            // ── Seleção por palavras ──────────────────────────────────────
            const int from = qMin(selStart, selEnd);
            const int to   = qMax(selStart, selEnd);
            QString text; QList<QRectF> pageRects;
            for (int i = from; i <= to && i < pageWords.size(); i++) {
                if (!text.isEmpty()) text += ' ';
                text += pageWords[i].word;
                pageRects.append(pageWords[i].bbox);
            }
            hasSel = false; selStart = selEnd = -1; update();
            if (!text.trimmed().isEmpty())
                emit selectionFinished(text.trimmed(), pageRects);

        } else {
            // ── Rubber-band: rect em coords de página ─────────────────────
            const QRect wr = QRect(pressPos, dragPos).normalized();
            hasSel = false; update();
            if (wr.width() > 8 && wr.height() > 4) {
                const QRectF pr = widgetToPage(QRectF(wr));
                emit selectionFinished(QString(), QList<QRectF>{ pr });
            }
        }
    } else {
        hasSel = false; selStart = selEnd = -1; update();
    }
}

QString PageWidget::selectedText() const
{
    if (!hasSel || selStart < 0 || pageWords.isEmpty()) return {};
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
    if (!hasSel || selStart < 0 || pageWords.isEmpty()) return rects;
    const int from = qMin(selStart, selEnd);
    const int to   = qMax(selStart, selEnd);
    for (int i = from; i <= to && i < pageWords.size(); i++)
        rects.append(pageWords[i].bbox);
    return rects;
}
