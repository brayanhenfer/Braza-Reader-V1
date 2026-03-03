#pragma once
// Extrai palavras de texto de uma página PDF usando MuPDF.
// O include de mupdf/fitz.h fica APENAS no .cpp para não contaminar outros headers.

#include <QList>
#include <QSizeF>

// TextWord está definido em pagewidget.h — mas TextExtractor.h é incluído
// SOMENTE em readerscreen.cpp, onde pagewidget.h já foi incluído antes.
// Para quebrar a dependência circular, usamos forward declaration do struct.
struct TextWord;

class TextExtractor
{
public:
    // ctx = PDFLoader::getContext(), doc = PDFLoader::getDocument()
    // Ambos são fz_context* / fz_document* mas passados como void* para
    // evitar incluir mupdf/fitz.h neste header.
    static QList<TextWord> extractWords(void* ctx, void* doc, int pageNumber);
    static QSizeF          pageSize    (void* ctx, void* doc, int pageNumber);
};
