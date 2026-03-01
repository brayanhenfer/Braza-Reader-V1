#pragma once

#include <QObject>
#include <QTouchEvent>
#include <QPoint>

class TouchHandler : public QObject
{
    Q_OBJECT

public:
    TouchHandler(QObject* parent = nullptr);
    ~TouchHandler();

    void handleTouchEvent(QTouchEvent* event);
    bool isSwipeGesture(const QPoint& startPos, const QPoint& endPos) const;

signals:
    void swipeUp();
    void swipeDown();
    void swipeLeft();
    void swipeRight();
    void tap(QPoint position);

private:
    QPoint lastTouchPos;
    const int SWIPE_THRESHOLD = 50;
};
