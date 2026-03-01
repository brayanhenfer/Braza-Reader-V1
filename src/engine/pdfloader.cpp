#include "pdfloader.h"
#include <QDebug>

PDFLoader::PDFLoader()
    : ctx(nullptr)
    , doc(nullptr)
    , pageCount(0)
    , documentOpen(false)
{
    ctx = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
    if (ctx) {
        fz_register_document_handlers(ctx);
    } else {
        qWarning() << "PDFLoader: Falha ao criar contexto MuPDF";
    }
}

PDFLoader::~PDFLoader()
{
    closeDocument();
    if (ctx) {
        fz_drop_context(ctx);
        ctx = nullptr;
    }
}

bool PDFLoader::openDocument(const QString& filePath)
{
    if (!ctx) return false;

    closeDocument();

    fz_try(ctx)
    {
        doc = fz_open_document(ctx, filePath.toUtf8().constData());
        pageCount = fz_count_pages(ctx, doc);
        documentOpen = true;
    }
    fz_catch(ctx)
    {
        qWarning() << "PDFLoader: Falha ao abrir documento:" << filePath;
        doc = nullptr;
        pageCount = 0;
        documentOpen = false;
        return false;
    }

    return true;
}

void PDFLoader::closeDocument()
{
    if (doc && ctx) {
        fz_drop_document(ctx, doc);
        doc = nullptr;
    }
    pageCount = 0;
    documentOpen = false;
}

int PDFLoader::getPageCount() const
{
    return pageCount;
}

bool PDFLoader::isOpen() const
{
    return documentOpen;
}

fz_context* PDFLoader::getContext() const
{
    return ctx;
}

fz_document* PDFLoader::getDocument() const
{
    return doc;
}