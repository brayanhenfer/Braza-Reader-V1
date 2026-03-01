#include "pdfcache.h"

PDFCache::PDFCache(int maxPages)
    : maxCacheSize(maxPages)
{
}

PDFCache::~PDFCache()
{
    clearCache();
}

void PDFCache::cachePageImage(int pageNumber, const QPixmap& image)
{
    if (cache.contains(pageNumber)) {
        accessOrder.removeAll(pageNumber);
        accessOrder.append(pageNumber);
        cache[pageNumber] = image;
        return;
    }

    while (cache.size() >= maxCacheSize) {
        evictOldest();
    }

    cache.insert(pageNumber, image);
    accessOrder.append(pageNumber);
}

QPixmap* PDFCache::getCachedPage(int pageNumber)
{
    if (!cache.contains(pageNumber)) {
        return nullptr;
    }

    accessOrder.removeAll(pageNumber);
    accessOrder.append(pageNumber);

    return &cache[pageNumber];
}

bool PDFCache::hasPage(int pageNumber) const
{
    return cache.contains(pageNumber);
}

void PDFCache::clearCache()
{
    cache.clear();
    accessOrder.clear();
}

void PDFCache::setMaxCacheSize(int maxPages)
{
    maxCacheSize = maxPages;
    while (cache.size() > maxCacheSize) {
        evictOldest();
    }
}

int PDFCache::cacheSize() const
{
    return cache.size();
}

void PDFCache::evictOldest()
{
    if (accessOrder.isEmpty()) return;

    int oldest = accessOrder.takeFirst();
    cache.remove(oldest);
}