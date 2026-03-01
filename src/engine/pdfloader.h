#pragma once

#include <QString>

extern "C" {
#include <mupdf/fitz.h>
}

class PDFLoader
{
public:
    PDFLoader();
    ~PDFLoader();

    bool openDocument(const QString& filePath);
    void closeDocument();

    int getPageCount() const;
    bool isOpen() const;

    fz_context* getContext() const;
    fz_document* getDocument() const;

private:
    fz_context* ctx;
    fz_document* doc;
    int pageCount;
    bool documentOpen;
};