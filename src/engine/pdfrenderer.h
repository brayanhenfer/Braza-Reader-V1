#pragma once

#include <QString>
#include <QPixmap>
#include <memory>

class PDFLoader;

class PDFRenderer
{
public:
    PDFRenderer();
    ~PDFRenderer();

    bool    openPDF(const QString& filePath);
    void    closePDF();
    QPixmap renderPage(int pageNumber, int width, int height);
    QPixmap renderThumbnail(int pageNumber, int thumbWidth, int thumbHeight);
    int     getPageCount() const;
    bool    isOpen() const;

private:
    std::unique_ptr<PDFLoader> loader;
};
