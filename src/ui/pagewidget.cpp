#include "pagewidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QTouchEvent>
#include <cmath>

PageWidget::PageWidget(QWidget* p) : QLabel(p){
    setAlignment(Qt::AlignCenter);
    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents, true);
}

void PageWidget::setPagePixmap(const QPixmap& px){ setPixmap(px); update(); }
void PageWidget::setAmberIntensity(int v)        { amberIntensity = v; update(); }
void PageWidget::setSepiaEnabled(bool e)         { sepiaEnabled = e; update(); }
void PageWidget::setHighlights(const QList<PageHighlight>& hl){ highlights = hl; update(); }
void PageWidget::setPageWords(const QList<TextWord>& w)       { pageWords = w; }
void PageWidget::setRenderScale(float sx, float sy)           { scaleX = sx; scaleY = sy; }

QRectF PageWidget::pageToWidget(const QRectF& r) const {
    return QRectF(r.x()*scaleX, r.y()*scaleY, r.width()*scaleX, r.height()*scaleY);
}
QRectF PageWidget::widgetToPage(const QRectF& r) const {
    if (scaleX <= 0 || scaleY <= 0) return r;
    return QRectF(r.x()/scaleX, r.y()/scaleY, r.width()/scaleX, r.height()/scaleY);
}

QRectF PageWidget::balloonRect(const PageHighlight& hl) const {
    if (hl.pageRects.isEmpty()) return {};
    const QRectF wr = pageToWidget(hl.pageRects.first());
    return QRectF(wr.right()+2, wr.top()-4, 22, 22);
}

int PageWidget::hitTestBalloon(const QPoint& pt) const {
    for (const PageHighlight& hl : highlights){
        if (hl.type != PageMarkType::Note) continue;
        if (balloonRect(hl).contains(QPointF(pt))) return hl.annotationId;
    }
    return -1;
}

// ── Pintura ───────────────────────────────────────────────────────────────────
void PageWidget::paintEvent(QPaintEvent* e){
    QLabel::paintEvent(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Só pinta balões de notas (highlight e underline removidos)
    for (const PageHighlight& hl : highlights){
        if (hl.pageRects.isEmpty()) continue;
        if (hl.type != PageMarkType::Note) continue;

        // Grifo leve sob a nota
        QColor c = hl.color; c.setAlpha(80);
        p.setBrush(c); p.setPen(Qt::NoPen);
        for (const QRectF& r : hl.pageRects)
            p.drawRect(pageToWidget(r));

        // Balão clicável
        const QRectF br = balloonRect(hl);
        if (!br.isNull()){
            // Sombra
            p.setBrush(QColor(0,0,0,55)); p.setPen(Qt::NoPen);
            p.drawEllipse(br.adjusted(2,2,2,2));
            // Círculo
            p.setBrush(hl.color); p.setPen(QPen(Qt::white, 1));
            p.drawEllipse(br);
            // Cauda
            QPainterPath tail;
            tail.moveTo(br.left()+4,  br.bottom()-2);
            tail.lineTo(br.left()-4,  br.bottom()+4);
            tail.lineTo(br.left()+10, br.bottom()-2);
            tail.closeSubpath();
            p.setPen(Qt::NoPen); p.setBrush(hl.color);
            p.drawPath(tail);
            // Três pontinhos
            p.setPen(QPen(Qt::white, 1.5));
            const double cy = br.center().y(), cx = br.center().x();
            for (int i = -1; i <= 1; i++)
                p.drawEllipse(QPointF(cx + i*4, cy), 1.4, 1.4);
        }
    }

    // Seleção ativa (word-based)
    if (hasSel && selStart >= 0 && selEnd >= 0 && !pageWords.isEmpty()){
        p.setBrush(QColor(80,140,230,100));
        p.setPen(QPen(QColor(60,110,200,180), 1));
        const int from = qMin(selStart,selEnd), to = qMax(selStart,selEnd);
        for (int i = from; i <= to && i < pageWords.size(); i++)
            p.drawRect(pageToWidget(pageWords[i].bbox));
    }
    // Rubber-band
    if (hasSel && pageWords.isEmpty() && isDragging){
        const QRect rr = QRect(pressPos, dragPos).normalized();
        p.setBrush(QColor(80,140,230,65));
        p.setPen(QPen(QColor(60,110,200,220), 2, Qt::DashLine));
        p.drawRect(rr);
    }

    // Filtros de leitura
    if (sepiaEnabled){
        p.setBrush(QColor(120,80,20,55)); p.setPen(Qt::NoPen); p.drawRect(rect());
    }
    if (amberIntensity > 0){
        p.setBrush(QColor(255,140,0, qBound(0, qRound(amberIntensity*2.0), 220)));
        p.setPen(Qt::NoPen); p.drawRect(rect());
    }
}

// ── Mouse ─────────────────────────────────────────────────────────────────────
static int nearestWordIdx(const QList<TextWord>& words, const QPoint& pt, float sx, float sy){
    int best = -1; double bestD = 1e9;
    for (int i = 0; i < words.size(); i++){
        const QRectF wr(words[i].bbox.x()*sx, words[i].bbox.y()*sy,
                        words[i].bbox.width()*sx, words[i].bbox.height()*sy);
        double d = qAbs(wr.center().x()-pt.x()) + qAbs(wr.center().y()-pt.y())*2.0;
        if (d < bestD){ bestD = d; best = i; }
    }
    return best;
}

void PageWidget::mousePressEvent(QMouseEvent* e){
    pressPos = e->pos(); dragPos = e->pos();
    isDragging = false; hasSel = false; selStart = selEnd = -1;
    tappedBalloonId = hitTestBalloon(e->pos());
    update();
}

void PageWidget::mouseMoveEvent(QMouseEvent* e){
    if (!(e->buttons() & Qt::LeftButton)) return;
    if ((e->pos()-pressPos).manhattanLength() > 8){
        isDragging = true; tappedBalloonId = -1;
    }
    if (isDragging){
        dragPos = e->pos();
        if (!pageWords.isEmpty()){
            selStart = nearestWordIdx(pageWords, pressPos, scaleX, scaleY);
            selEnd   = nearestWordIdx(pageWords, dragPos,  scaleX, scaleY);
        }
        hasSel = true; update();
    }
}

void PageWidget::mouseReleaseEvent(QMouseEvent*){
    const bool wasDragging = isDragging; isDragging = false;

    if (!wasDragging){
        const int ballId = tappedBalloonId; tappedBalloonId = -1;
        hasSel = false; selStart = selEnd = -1; update();
        if (ballId >= 0){
            QString noteText;
            for (const PageHighlight& hl : highlights)
                if (hl.annotationId == ballId){ noteText = hl.notePreview; break; }
            emit noteMarkerTapped(ballId, noteText);
        } else {
            emit pageTapped();
        }
        return;
    }

    if (hasSel){
        if (!pageWords.isEmpty()){
            const int from = qMin(selStart,selEnd), to = qMax(selStart,selEnd);
            QString text; QList<QRectF> pageRects;
            for (int i = from; i <= to && i < pageWords.size(); i++){
                if (!text.isEmpty()) text += ' ';
                text += pageWords[i].word;
                pageRects.append(pageWords[i].bbox);
            }
            hasSel = false; selStart = selEnd = -1; update();
            if (!text.trimmed().isEmpty())
                emit selectionFinished(text.trimmed(), pageRects);
        } else {
            const QRect wr = QRect(pressPos, dragPos).normalized();
            hasSel = false; update();
            if (wr.width() > 8 && wr.height() > 4)
                emit selectionFinished(QString(), {widgetToPage(QRectF(wr))});
        }
    } else {
        hasSel = false; selStart = selEnd = -1; update();
        emit pageTapped();
    }
}

QString PageWidget::selectedText() const {
    const int from = qMin(selStart,selEnd), to = qMax(selStart,selEnd);
    QString t;
    for (int i = from; i <= to && i < pageWords.size(); i++){
        if (!t.isEmpty()) t += ' ';
        t += pageWords[i].word;
    }
    return t;
}
QList<QRectF> PageWidget::selectedPageRects() const {
    QList<QRectF> r;
    if (!hasSel || selStart < 0 || pageWords.isEmpty()) return r;
    const int from = qMin(selStart,selEnd), to = qMax(selStart,selEnd);
    for (int i = from; i <= to && i < pageWords.size(); i++)
        r.append(pageWords[i].bbox);
    return r;
}

// ── Touch ─────────────────────────────────────────────────────────────────────
bool PageWidget::event(QEvent* e){
    switch (e->type()){
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        auto* te = static_cast<QTouchEvent*>(e);
        if (te->touchPoints().isEmpty()) return true;
        const QTouchEvent::TouchPoint& tp = te->touchPoints().first();
        const QPoint pos = tp.pos().toPoint();
        if (e->type() == QEvent::TouchBegin){
            QMouseEvent me(QEvent::MouseButtonPress,   pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            mousePressEvent(&me);
        } else if (e->type() == QEvent::TouchUpdate){
            QMouseEvent me(QEvent::MouseMove,           pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            mouseMoveEvent(&me);
        } else {
            QMouseEvent me(QEvent::MouseButtonRelease,  pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
            mouseReleaseEvent(&me);
        }
        e->accept(); return true;
    }
    default: return QLabel::event(e);
    }
}
