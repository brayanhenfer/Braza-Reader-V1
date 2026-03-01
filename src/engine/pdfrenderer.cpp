#include "pdfrenderer.h"
#include "pdfloader.h"
#include <QImage>
#include <QDebug>

extern "C" {
#include <mupdf/fitz.h>
}

PDFRenderer::PDFRenderer()
    : loader(std::make_unique<PDFLoader>())
{
}

PDFRenderer::~PDFRenderer()
{
    closePDF();
}

bool PDFRenderer::openPDF(const QString& filePath)
{
    return loader->openDocument(filePath);
}

void PDFRenderer::closePDF()
{
    loader->closeDocument();
}

int PDFRenderer::getPageCount() const
{
    return loader->getPageCount();
}

bool PDFRenderer::isOpen() const
{
    return loader->isOpen();
}

QPixmap PDFRenderer::renderPage(int pageNumber, int width, int height)
{
    if (!loader->isOpen()) return QPixmap();

    fz_context* ctx = loader->getContext();
    fz_document* doc = loader->getDocument();

    if (!ctx || !doc) return QPixmap();
    if (pageNumber < 0 || pageNumber >= loader->getPageCount()) return QPixmap();

    fz_page* page = nullptr;
    fz_pixmap* pix = nullptr;
    QPixmap result;

    fz_try(ctx)
    {
        page = fz_load_page(ctx, doc, pageNumber);

        // Calcular escala para caber no tamanho desejado
        fz_rect bounds = fz_bound_page(ctx, page);
        float pageWidth = bounds.x1 - bounds.x0;
        float pageHeight = bounds.y1 - bounds.y0;

        float scaleX = (float)width / pageWidth;
        float scaleY = (float)height / pageHeight;
        float scale = (scaleX < scaleY) ? scaleX : scaleY;

        fz_matrix ctm = fz_scale(scale, scale);

        // Renderizar para pixmap RGB
        pix = fz_new_pixmap_from_page(ctx, page, ctm, fz_device_rgb(ctx), 0);

        int pixWidth = fz_pixmap_width(ctx, pix);
        int pixHeight = fz_pixmap_height(ctx, pix);
        unsigned char* samples = fz_pixmap_samples(ctx, pix);
        int stride = fz_pixmap_stride(ctx, pix);

        // Converter para QImage e depois QPixmap
        QImage image(samples, pixWidth, pixHeight, stride, QImage::Format_RGB888);
        result = QPixmap::fromImage(image.copy());
    }
    fz_always(ctx)
    {
        if (pix) fz_drop_pixmap(ctx, pix);
        if (page) fz_drop_page(ctx, page);
    }
    fz_catch(ctx)
    {
        qWarning() << "PDFRenderer: Falha ao renderizar pagina" << pageNumber;
        return QPixmap();
    }

    return result;
}