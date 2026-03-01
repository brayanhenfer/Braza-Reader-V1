#include "touchhandler.h"
#include <cmath>

TouchHandler::TouchHandler(QObject* parent)
    : QObject(parent)
{
}

TouchHandler::~TouchHandler() = default;

void TouchHandler::handleTouchEvent(QTouchEvent* event)
{
    if (event->type() == QEvent::TouchBegin) {
        if (event->touchPoints().size() == 1) {
            lastTouchPos = event->touchPoints().first().pos().toPoint();
        }
    } else if (event->type() == QEvent::TouchEnd) {
        if (event->touchPoints().size() == 1) {
            QPoint currentPos = event->touchPoints().first().pos().toPoint();

            if (isSwipeGesture(lastTouchPos, currentPos)) {
                int deltaX = currentPos.x() - lastTouchPos.x();
                int deltaY = currentPos.y() - lastTouchPos.y();

                if (std::abs(deltaY) > std::abs(deltaX)) {
                    if (deltaY > 0) {
                        emit swipeDown();
                    } else {
                        emit swipeUp();
                    }
                } else {
                    if (deltaX > 0) {
                        emit swipeRight();
                    } else {
                        emit swipeLeft();
                    }
                }
            } else {
                emit tap(currentPos);
            }
        }
    }
}

bool TouchHandler::isSwipeGesture(const QPoint& startPos, const QPoint& endPos) const
{
    int deltaX = std::abs(endPos.x() - startPos.x());
    int deltaY = std::abs(endPos.y() - startPos.y());
    return (deltaX >= SWIPE_THRESHOLD || deltaY >= SWIPE_THRESHOLD);
}
