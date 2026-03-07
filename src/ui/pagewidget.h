#pragma once
#include <QLabel>
#include <QPoint>
#include <QRectF>
#include <QColor>
#include <QList>
#include <QPixmap>
#include <QString>
#include <QEvent>

struct TextWord { QRectF bbox; QString word; };

// Apenas Note é usado agora (Highlight e Underline removidos da UI)
enum class PageMarkType { Highlight = 0, Note = 1, Bookmark = 2, Underline = 3 };

struct PageHighlight {
    QList<QRectF> pageRects;
    QColor        color;
    int           annotationId = -1;
    PageMarkType  type = PageMarkType::Note;
    QString       notePreview;
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
    void selectionFinished(const QString& text, const QList<QRectF>& pageRects);
    void pageTapped();
    void noteMarkerTapped(int annotationId, const QString& noteText);

protected:
    bool event(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QRectF pageToWidget(const QRectF& r) const;
    QRectF widgetToPage(const QRectF& r) const;
    QRectF balloonRect(const PageHighlight& hl) const;
    int    hitTestBalloon(const QPoint& pt) const;

    QPoint pressPos, dragPos;
    bool   isDragging = false;
    bool   hasSel     = false;
    int    amberIntensity = 0;
    bool   sepiaEnabled   = false;
    float  scaleX = 1.f, scaleY = 1.f;
    QList<TextWord>      pageWords;
    int selStart = -1, selEnd = -1;
    QList<PageHighlight> highlights;
    int tappedBalloonId = -1;
};
