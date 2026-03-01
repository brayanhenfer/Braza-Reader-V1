#pragma once

#include <QPixmap>
#include <QMap>
#include <QList>

class PDFCache
{
public:
    explicit PDFCache(int maxPages = 5);
    ~PDFCache();

    void cachePageImage(int pageNumber, const QPixmap& image);
    QPixmap* getCachedPage(int pageNumber);
    bool hasPage(int pageNumber) const;
    void clearCache();
    void setMaxCacheSize(int maxPages);
    int cacheSize() const;

private:
    void evictOldest();

    int maxCacheSize;
    QMap<int, QPixmap> cache;
    QList<int> accessOrder;
};