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

void PageWidget::setPagePixmap(const QPixmap& px)  { setPixmap(px); update(); }
void PageWidget::setAmberIntensity(int v)           { amberIntensity = v; update(); }
void PageWidget::setSepiaEnabled(bool e)            { sepiaEnabled = e; update(); }
void PageWidget::setHighlights(const QList<PageHighlight>& hl) { highlights = hl; update(); }
void PageWidget::setPageWords(const QList<TextWord>& w) { pageWords = w; }
void PageWidget::setRenderScale(float sx, float sy) { scaleX = sx; scaleY = sy; }

// PageWidget usa setFixedSize(scaled.size()), portanto pixmap ocupa tudo.
// Offset é sempre (0,0). Mantemos o método por segurança.
QPoint PageWidget::pixmapOffset() const { return QPoint(0, 0); }

QRectF PageWidget::pageToWidget(const QRectF& r) const
{
    return QRectF(r.x() * scaleX, r.y() * scaleY,
                  r.width() * scaleX, r.height() * scaleY);
}

// Converte coords de widget para página
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
        c.setAlpha(100);
        p.setBrush(c);
        p.setPen(Qt::NoPen);
        for (const QRectF& r : hl.pageRects)
            p.drawRect(pageToWidget(r));
    }

    // 2a. Seleção por palavras (quando há extração de texto)
    if (hasSel && selStart >= 0 && selEnd >= 0 && !pageWords.isEmpty()) {
        p.setBrush(QColor(100, 149, 237, 100));
        p.setPen(QPen(QColor(60, 100, 220, 160), 1));
        const int from = qMin(selStart, selEnd);
        const int to   = qMax(selStart, selEnd);
        for (int i = from; i <= to && i < pageWords.size(); i++)
            p.drawRect(pageToWidget(pageWords[i].bbox));
    }

    // 2b. Seleção rubber-band (quando NÃO há extração de texto)
    if (hasSel && pageWords.isEmpty() && isDragging) {
        QRect rr = QRect(pressPos, dragPos).normalized();
        p.setBrush(QColor(100, 149, 237, 70));
        p.setPen(QPen(QColor(60, 100, 220, 220), 2, Qt::DashLine));
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
    int    best = -1;
    double bestD = 1e9;
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
    pressTimer.stop();
    const bool wasDragging = isDragging;
    isDragging = false;

    if (wasDragging && hasSel) {

        if (!pageWords.isEmpty()) {
            // ── Seleção por palavras ──────────────────────────────────────
            const int from = qMin(selStart, selEnd);
            const int to   = qMax(selStart, selEnd);
            QString       text;
            QList<QRectF> pageRects;
            for (int i = from; i <= to && i < pageWords.size(); i++) {
                if (!text.isEmpty()) text += ' ';
                text += pageWords[i].word;
                pageRects.append(pageWords[i].bbox);
            }
            hasSel = false; selStart = selEnd = -1; update();
            if (!text.trimmed().isEmpty())
                emit selectionFinished(text.trimmed(), pageRects);

        } else {
            // ── Rubber-band: converte rect de widget → página ─────────────
            const QRect  widgetRect = QRect(pressPos, dragPos).normalized();
            if (widgetRect.width() > 10 && widgetRect.height() > 5) {
                const QRectF pageRect = widgetToPage(QRectF(widgetRect));
                hasSel = false; update();
                emit selectionFinished("[grifo manual]", { pageRect });
            } else {
                hasSel = false; update();
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
