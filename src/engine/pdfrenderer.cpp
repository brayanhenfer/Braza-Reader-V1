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

// ─────────────────────────────────────────────────────────────────────────────
// renderPage  –  CORRIGIDO: renderiza em alta resolução e depois escala
//
// PROBLEMA ANTERIOR:
//   scale = targetWidth / pageWidth  →  para miniaturas era ~0.2x
//   MuPDF gerava pixmap minúsculo → Qt esticava → BLUR
//
// SOLUÇÃO:
//   1. Calcula escala base para caber no alvo
//   2. Multiplica por OVERSAMPLE_FACTOR (2x) para renderizar maior
//   3. Qt reduz com SmoothTransformation → imagem nítida
//
// Para leitura em tela cheia (1024x600) o fator garante que a primeira
// página não apareça embaçada mesmo em PDFs com pageWidth pequeno.
// ─────────────────────────────────────────────────────────────────────────────
QPixmap PDFRenderer::renderPage(int pageNumber, int width, int height)
{
    if (!loader->isOpen()) return QPixmap();

    fz_context*  ctx = loader->getContext();
    fz_document* doc = loader->getDocument();

    if (!ctx || !doc) return QPixmap();
    if (pageNumber < 0 || pageNumber >= loader->getPageCount()) return QPixmap();

    // Fator de supersample:
    //   - Tela 7" 1024x600 tem densidade ~170 dpi, mas o Qt trabalha em pixels lógicos.
    //   - Renderizar 2x e reduzir via SmoothTransformation elimina o blur.
    //   - Para miniaturas (width/height pequenos) o custo é baixo; para leitura
    //     em tela cheia é aceitável no Raspberry Pi Zero 2W.
    constexpr float OVERSAMPLE = 2.0f;

    fz_page*   page = nullptr;
    fz_pixmap* pix  = nullptr;
    QPixmap    result;

    fz_try(ctx)
    {
        page = fz_load_page(ctx, doc, pageNumber);

        fz_rect bounds    = fz_bound_page(ctx, page);
        float   pageWidth  = bounds.x1 - bounds.x0;
        float   pageHeight = bounds.y1 - bounds.y0;

        if (pageWidth <= 0 || pageHeight <= 0)
            fz_throw(ctx, FZ_ERROR_GENERIC, "invalid page dimensions");

        // Escala base para caber no rectângulo alvo
        float scaleX = (float)width  / pageWidth;
        float scaleY = (float)height / pageHeight;
        float scale  = (scaleX < scaleY) ? scaleX : scaleY;

        // Escala real de renderização: 2x o necessário
        float renderScale = scale * OVERSAMPLE;

        fz_matrix ctm = fz_scale(renderScale, renderScale);

        // Renderizar em RGB sem canal alpha (mais leve)
        pix = fz_new_pixmap_from_page(ctx, page, ctm, fz_device_rgb(ctx), 0);

        int           pixWidth  = fz_pixmap_width(ctx, pix);
        int           pixHeight = fz_pixmap_height(ctx, pix);
        unsigned char* samples  = fz_pixmap_samples(ctx, pix);
        int           stride    = fz_pixmap_stride(ctx, pix);

        // Construir QImage sem cópia extra (o fz_pixmap ainda está vivo aqui)
        QImage image(samples, pixWidth, pixHeight, stride, QImage::Format_RGB888);

        // Dimensões finais (o que o caller pediu)
        int finalW = qRound(pageWidth  * scale);
        int finalH = qRound(pageHeight * scale);

        // Reduzir com filtro bilinear suave → sem blur, sem serrilhado
        QImage scaled = image.scaled(finalW, finalH,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);

        result = QPixmap::fromImage(scaled);
    }
    fz_always(ctx)
    {
        if (pix)  fz_drop_pixmap(ctx, pix);
        if (page) fz_drop_page(ctx, page);
    }
    fz_catch(ctx)
    {
        qWarning() << "PDFRenderer: Falha ao renderizar pagina" << pageNumber;
        return QPixmap();
    }

    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// renderThumbnail  –  versão otimizada para miniaturas da biblioteca
//
// Usa OVERSAMPLE menor (1.5x) porque miniaturas são pequenas e o custo
// de renderizar 2x seria desperdiçado para imagens de 120x160 px.
// ─────────────────────────────────────────────────────────────────────────────
QPixmap PDFRenderer::renderThumbnail(int pageNumber, int thumbWidth, int thumbHeight)
{
    if (!loader->isOpen()) return QPixmap();

    fz_context*  ctx = loader->getContext();
    fz_document* doc = loader->getDocument();

    if (!ctx || !doc) return QPixmap();
    if (pageNumber < 0 || pageNumber >= loader->getPageCount()) return QPixmap();

    constexpr float OVERSAMPLE_THUMB = 1.5f;

    fz_page*   page = nullptr;
    fz_pixmap* pix  = nullptr;
    QPixmap    result;

    fz_try(ctx)
    {
        page = fz_load_page(ctx, doc, pageNumber);

        fz_rect bounds    = fz_bound_page(ctx, page);
        float   pageWidth  = bounds.x1 - bounds.x0;
        float   pageHeight = bounds.y1 - bounds.y0;

        if (pageWidth <= 0 || pageHeight <= 0)
            fz_throw(ctx, FZ_ERROR_GENERIC, "invalid page dimensions");

        float scaleX = (float)thumbWidth  / pageWidth;
        float scaleY = (float)thumbHeight / pageHeight;
        float scale  = (scaleX < scaleY) ? scaleX : scaleY;

        float renderScale = scale * OVERSAMPLE_THUMB;

        fz_matrix ctm = fz_scale(renderScale, renderScale);

        pix = fz_new_pixmap_from_page(ctx, page, ctm, fz_device_rgb(ctx), 0);

        int           pixWidth  = fz_pixmap_width(ctx, pix);
        int           pixHeight = fz_pixmap_height(ctx, pix);
        unsigned char* samples  = fz_pixmap_samples(ctx, pix);
        int           stride    = fz_pixmap_stride(ctx, pix);

        QImage image(samples, pixWidth, pixHeight, stride, QImage::Format_RGB888);

        int finalW = qRound(pageWidth  * scale);
        int finalH = qRound(pageHeight * scale);

        QImage scaled = image.scaled(finalW, finalH,
                                     Qt::KeepAspectRatio,
                                     Qt::SmoothTransformation);

        result = QPixmap::fromImage(scaled);
    }
    fz_always(ctx)
    {
        if (pix)  fz_drop_pixmap(ctx, pix);
        if (page) fz_drop_page(ctx, page);
    }
    fz_catch(ctx)
    {
        qWarning() << "PDFRenderer: Falha ao renderizar thumbnail pagina" << pageNumber;
        return QPixmap();
    }

    return result;
}