// textextractor.cpp — inclui mupdf/fitz.h AQUI, não no header
#include "textextractor.h"
#include "../ui/pagewidget.h"   // define TextWord

extern "C" {
#include <mupdf/fitz.h>
}

#include <algorithm>
#include <QDebug>

QList<TextWord> TextExtractor::extractWords(void* vctx, void* vdoc, int pageNumber)
{
    QList<TextWord> result;
    fz_context*  ctx = static_cast<fz_context*>(vctx);
    fz_document* doc = static_cast<fz_document*>(vdoc);
    if (!ctx || !doc || pageNumber < 0) return result;

    fz_page*       page  = nullptr;
    fz_stext_page* spage = nullptr;

    fz_try(ctx) {
        page = fz_load_page(ctx, doc, pageNumber);
        fz_stext_options opts{};
        spage = fz_new_stext_page_from_page(ctx, page, &opts);

        TextWord current;

        for (fz_stext_block* blk = spage->first_block; blk; blk = blk->next) {
            if (blk->type != FZ_STEXT_BLOCK_TEXT) continue;
            for (fz_stext_line* ln = blk->u.t.first_line; ln; ln = ln->next) {
                for (fz_stext_char* ch = ln->first_char; ch; ch = ch->next) {

                    auto flush = [&]() {
                        if (!current.word.isEmpty()) {
                            result.append(current);
                            current.word.clear();
                            current.bbox = {};
                        }
                    };

                    if (ch->c == ' ' || ch->c == '\t' || ch->c == '\n') {
                        flush();
                        continue;
                    }

                    // Bounding box a partir do quad do caracter
                    const float x0 = std::min({ch->quad.ul.x, ch->quad.ur.x,
                                           ch->quad.ll.x, ch->quad.lr.x});
                    const float y0 = std::min({ch->quad.ul.y, ch->quad.ur.y,
                                           ch->quad.ll.y, ch->quad.lr.y});
                    const float x1 = std::max({ch->quad.ul.x, ch->quad.ur.x,
                                           ch->quad.ll.x, ch->quad.lr.x});
                    const float y1 = std::max({ch->quad.ul.y, ch->quad.ur.y,
                                           ch->quad.ll.y, ch->quad.lr.y});
                    const QRectF cbox(x0, y0, x1 - x0, y1 - y0);

                    current.bbox = current.word.isEmpty()
                                   ? cbox
                                   : current.bbox.united(cbox);

                    uint uch = static_cast<uint>(ch->c);
                    current.word += QString::fromUcs4(&uch, 1);
                }
                // Flush ao fim de cada linha
                if (!current.word.isEmpty()) {
                    result.append(current);
                    current.word.clear();
                    current.bbox = {};
                }
            }
        }
    }
    fz_always(ctx) {
        if (spage) fz_drop_stext_page(ctx, spage);
        if (page)  fz_drop_page(ctx, page);
    }
    fz_catch(ctx) {
        qWarning() << "TextExtractor::extractWords: failed page" << pageNumber;
    }
    return result;
}

QSizeF TextExtractor::pageSize(void* vctx, void* vdoc, int pageNumber)
{
    fz_context*  ctx = static_cast<fz_context*>(vctx);
    fz_document* doc = static_cast<fz_document*>(vdoc);
    if (!ctx || !doc || pageNumber < 0) return {};

    fz_page* page = nullptr;
    QSizeF   sz;
    fz_try(ctx) {
        page = fz_load_page(ctx, doc, pageNumber);
        const fz_rect b = fz_bound_page(ctx, page);
        sz = QSizeF(static_cast<double>(b.x1 - b.x0),
                    static_cast<double>(b.y1 - b.y0));
    }
    fz_always(ctx) { if (page) fz_drop_page(ctx, page); }
    fz_catch(ctx)  { qWarning() << "TextExtractor::pageSize: failed" << pageNumber; }
    return sz;
}
