#include "readerscreen.h"
#include "topbar_helper.h"
#include "../engine/pdfrenderer.h"
#include "../engine/pdfloader.h"
#include "../engine/textextractor.h"
#include "../storage/progressmanager.h"
#include "../storage/annotationmanager.h"
#include "../storage/settingsmanager.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QFileInfo>
#include <QDialog>
#include <QTextEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollBar>
#include <QTimer>

ReaderScreen::ReaderScreen(QWidget* parent)
    : QWidget(parent)
    , renderer(std::make_unique<PDFRenderer>())
    , progressManager(std::make_unique<ProgressManager>())
    , annotManager(std::make_unique<AnnotationManager>())
{
    SettingsManager sm;
    amberIntensity = sm.getAmberIntensity();
    sepiaEnabled   = sm.getSepiaEnabled();
    setupUI();
    setFocusPolicy(Qt::StrongFocus);
}
ReaderScreen::~ReaderScreen() { closeBook(); }

// ── Setup ─────────────────────────────────────────────────────────────────────
void ReaderScreen::setupUI(){
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    setupTopBar();
    setupScrollArea();
    setupBottomBar();
}

void ReaderScreen::setupTopBar(){
    backButton = new QPushButton("←");
    backButton->setFixedSize(40,40);
    backButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:22px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(backButton, &QPushButton::clicked, this, &ReaderScreen::backClicked);

    auto* annotBtn = new QPushButton(QString::fromUtf8("✏"));
    annotBtn->setFixedSize(36,36);
    annotBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:18px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:18px;}");
    connect(annotBtn, &QPushButton::clicked, this, &ReaderScreen::onAddAnnotation);

    topBar = TopBarHelper::create(this, backButton, annotBtn);
    mainLayout->addWidget(topBar);
}

void ReaderScreen::setupScrollArea(){
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea{border:none;background:#1a1a1a;}");
    scrollArea->viewport()->setMouseTracking(true);

    pagesContainer = new QWidget();
    pagesContainer->setStyleSheet("background:#1a1a1a;");
    pagesLayout = new QVBoxLayout(pagesContainer);
    pagesLayout->setContentsMargins(0,12,0,12);
    pagesLayout->setSpacing(10);
    pagesLayout->setAlignment(Qt::AlignHCenter);
    scrollArea->setWidget(pagesContainer);

    scrollDebounce = new QTimer(this);
    scrollDebounce->setSingleShot(true);
    scrollDebounce->setInterval(80);
    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int){ scrollDebounce->start(); });
    connect(scrollDebounce, &QTimer::timeout, this, &ReaderScreen::onScrollDebounced);
    mainLayout->addWidget(scrollArea, 1);
}

void ReaderScreen::setupBottomBar(){
    bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(50);
    bottomBar->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #1a1a1a,stop:1 #111);"
        "border-top:1px solid #2a2a2a;");
    auto* lay = new QHBoxLayout(bottomBar);
    lay->setContentsMargins(12,5,12,5);
    lay->setSpacing(8);

    const QString btnS =
        "QPushButton{background:#2a2a2a;color:white;border-radius:18px;"
        "font-size:18px;border:1px solid #3a3a3a;}"
        "QPushButton:pressed{background:#444;}";

    zoomOutBtn = new QPushButton("−", bottomBar);
    zoomOutBtn->setFixedSize(36,36);
    zoomOutBtn->setStyleSheet(btnS);
    connect(zoomOutBtn, &QPushButton::clicked, this, &ReaderScreen::onZoomOut);

    pageInfoLabel = new QLabel("", bottomBar);
    pageInfoLabel->setStyleSheet("color:#aaa;font-size:13px;");
    pageInfoLabel->setAlignment(Qt::AlignCenter);

    zoomInBtn = new QPushButton("+", bottomBar);
    zoomInBtn->setFixedSize(36,36);
    zoomInBtn->setStyleSheet(btnS);
    connect(zoomInBtn, &QPushButton::clicked, this, &ReaderScreen::onZoomIn);

    auto* listBtn = new QPushButton(QString::fromUtf8("📋"), bottomBar);
    listBtn->setFixedSize(36,36);
    listBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:18px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.15);border-radius:18px;}");
    connect(listBtn, &QPushButton::clicked, this, &ReaderScreen::onShowAnnotationsList);

    lay->addWidget(zoomOutBtn);
    lay->addWidget(pageInfoLabel, 1);
    lay->addWidget(zoomInBtn);
    lay->addSpacing(6);
    lay->addWidget(listBtn);

    mainLayout->addWidget(bottomBar);
}

// ── Diálogo de anotação (usado tanto pelo ✏ como pela seleção de texto) ───────
void ReaderScreen::openNoteDialog(int pageIndex, const QString& selText,
                                   const QList<QRectF>& rects,
                                   int existingId, const QString& existingNote)
{
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle(existingId >= 0
        ? QString::fromUtf8("Editar Anotação")
        : QString::fromUtf8("Nova Anotação"));
    dlg->setStyleSheet("background:#1a1a1a;color:white;");
    dlg->resize(500, 340);

    auto* dl = new QVBoxLayout(dlg);
    dl->setContentsMargins(16,16,16,16);
    dl->setSpacing(10);

    // Preview do texto selecionado
    if (!selText.isEmpty()) {
        auto* prev = new QLabel(selText.left(150) + (selText.length()>150?"…":""), dlg);
        prev->setStyleSheet(
            "color:#ddd;font-size:11px;background:#252525;border-radius:5px;"
            "padding:8px;border-left:3px solid #FFD700;");
        prev->setWordWrap(true);
        prev->setMaximumHeight(70);
        dl->addWidget(prev);
    } else {
        auto* lbl = new QLabel(
            QString::fromUtf8("Página %1").arg(pageIndex+1), dlg);
        lbl->setStyleSheet("color:#aaa;font-size:12px;");
        dl->addWidget(lbl);
    }

    auto* te = new QTextEdit(dlg);
    te->setStyleSheet(
        "QTextEdit{background:#252525;color:white;border:1px solid #3a3a3a;"
        "border-radius:5px;padding:6px;font-size:13px;}");
    te->setPlaceholderText(QString::fromUtf8("Escreva sua anotação aqui..."));
    if (!existingNote.isEmpty()) te->setText(existingNote);
    dl->addWidget(te, 1);

    auto* bRow = new QHBoxLayout();
    auto* ok = new QPushButton(QString::fromUtf8("✓ Salvar"), dlg);
    ok->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:10px 20px;"
        "border-radius:6px;font-size:13px;font-weight:bold;}"
        "QPushButton:pressed{background:#2a8040;}");
    auto* cn = new QPushButton("Cancelar", dlg);
    cn->setStyleSheet(
        "QPushButton{background:#333;color:white;padding:10px;border-radius:6px;"
        "font-size:13px;}"
        "QPushButton:pressed{background:#555;}");
    connect(ok, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(cn, &QPushButton::clicked, dlg, &QDialog::reject);
    bRow->addWidget(cn);
    bRow->addWidget(ok);
    dl->addLayout(bRow);

    if (dlg->exec() == QDialog::Accepted) {
        const QString noteText = te->toPlainText().trimmed();
        if (existingId >= 0) {
            annotManager->updateNote(existingId, noteText);
        } else {
            const QString finalNote = noteText.isEmpty()
                ? QString::fromUtf8("(sem texto)") : noteText;
            annotManager->addNote(currentTitle, pageIndex, finalNote,
                                  selText, QColor(255,235,59), rects);
        }
        reloadPageHighlights(pageIndex);
    }
    dlg->deleteLater();
}

// ── Botão ✏ na topbar → nova nota na página atual ────────────────────────────
void ReaderScreen::onAddAnnotation(){
    if (!bookOpen) return;
    openNoteDialog(currentPage, QString(), QList<QRectF>());
}

// ── Abertura / Fechamento ─────────────────────────────────────────────────────
void ReaderScreen::openBook(const QString& filePath){
    closeBook();
    currentFilePath = filePath;
    currentTitle    = QFileInfo(filePath).completeBaseName();
    if (!renderer->openPDF(filePath)) return;
    bookOpen    = true;
    totalPages  = renderer->getPageCount();

    int savedPage = progressManager->getLastPage(currentTitle);
    if (savedPage < 0 || savedPage >= totalPages) savedPage = 0;
    currentPage = savedPage;

    topBar->show(); bottomBar->show(); topBarVisible = true;
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    windowStart = qMax(0, currentPage);
    QTimer::singleShot(0, this, [this](){ renderVisiblePages(); });
    setFocus();
}

void ReaderScreen::closeBook(){
    if (!bookOpen) return;
    if (!pageWidgets.isEmpty()){
        const int scrollY = scrollArea->verticalScrollBar()->value();
        int cumY = 0, calcPage = windowStart;
        for (int i = 0; i < pageWidgets.size(); i++){
            const int h = pageWidgets[i]->height() + 10;
            if (scrollY < cumY + h){ calcPage = windowStart + i; break; }
            cumY += h; calcPage = windowStart + i;
        }
        currentPage = calcPage;
    }
    progressManager->saveProgress(currentTitle, currentPage);
    renderer->closePDF();
    for (auto* w : pageWidgets){ pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();
    bookOpen = false; currentPage = 0; totalPages = 0;
    if (pageInfoLabel) pageInfoLabel->clear();
}

// ── Renderização ──────────────────────────────────────────────────────────────
void ReaderScreen::renderVisiblePages(){
    if (!bookOpen) return;
    for (auto* w : pageWidgets){ pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();

    int availW = scrollArea->viewport()->width();
    if (availW < 50){ QTimer::singleShot(20, this, &ReaderScreen::renderVisiblePages); return; }

    const int end = qMin(windowStart + MAX_LOADED, totalPages);
    auto textLoader = std::make_unique<PDFLoader>();
    const bool textOk = textLoader->openDocument(currentFilePath);

    for (int p = windowStart; p < end; p++){
        auto* pw = new PageWidget(pagesContainer);
        pw->setAmberIntensity(amberIntensity);
        pw->setSepiaEnabled(sepiaEnabled);

        const int rW = qRound(availW * zoomFactor);
        const int rH = qRound(rW * 1.414f);
        QPixmap pix = renderer->renderPage(p, rW, rH);

        if (!pix.isNull()){
            const QPixmap scaled = pix.scaled(rW, rH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            pw->setPagePixmap(scaled);
            pw->setFixedSize(scaled.size());
            if (textOk){
                const QSizeF ps = TextExtractor::pageSize(
                    textLoader->getContext(), textLoader->getDocument(), p);
                if (ps.width() > 0 && ps.height() > 0){
                    pw->setRenderScale(
                        float(scaled.width())  / float(ps.width()),
                        float(scaled.height()) / float(ps.height()));
                    pw->setPageWords(TextExtractor::extractWords(
                        textLoader->getContext(), textLoader->getDocument(), p));
                }
            }
        } else {
            pw->setText(QString("Página %1").arg(p+1));
            pw->setFixedSize(rW, rH);
        }

        loadHighlightsForPage(pw, p);
        const int pi = p;

        connect(pw, &PageWidget::pageTapped, this, &ReaderScreen::onPageTapped);

        // Seleção de texto → abre diálogo direto
        connect(pw, &PageWidget::selectionFinished, this,
            [this, pi](const QString& t, const QList<QRectF>& r){
                if (r.isEmpty()) return;
                openNoteDialog(pi, t, r);
            });

        // Tap no balão → editar nota existente
        connect(pw, &PageWidget::noteMarkerTapped, this,
            [this, pi](int annotId, const QString& noteText){
                openNoteDialog(pi, QString(), QList<QRectF>(), annotId, noteText);
            });

        pagesLayout->addWidget(pw, 0, Qt::AlignHCenter);
        pageWidgets.append(pw);
    }
    updatePageInfo();
}

void ReaderScreen::loadHighlightsForPage(PageWidget* pw, int pageIndex){
    const auto anns = annotManager->getAnnotationsForPage(currentTitle, pageIndex);
    QList<PageHighlight> hls;
    for (const Annotation& a : anns){
        if (a.type == Annotation::Bookmark) continue;
        if (a.type != Annotation::Note) continue;
        PageHighlight hl;
        hl.pageRects    = a.rects;
        hl.color        = a.highlightColor;
        hl.annotationId = a.id;
        hl.type         = PageMarkType::Note;
        hl.notePreview  = a.noteText;
        hls.append(hl);
    }
    pw->setHighlights(hls);
}

void ReaderScreen::reloadPageHighlights(int pageIndex){
    const int local = pageIndex - windowStart;
    if (local >= 0 && local < pageWidgets.size())
        loadHighlightsForPage(pageWidgets[local], pageIndex);
}

// ── Toggle Topbar ─────────────────────────────────────────────────────────────
void ReaderScreen::onPageTapped(){
    topBarVisible = !topBarVisible;
    topBar->setVisible(topBarVisible);
    bottomBar->setVisible(topBarVisible);
    scrollArea->setVerticalScrollBarPolicy(
        topBarVisible ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
}

// ── Lista de Anotações (📋) ───────────────────────────────────────────────────
void ReaderScreen::onShowAnnotationsList(){
    if (!bookOpen) return;
    auto* dlg = new QDialog(this);
    dlg->setWindowTitle(QString::fromUtf8("Anotações — ") + currentTitle);
    dlg->setStyleSheet("background:#1a1a1a;color:white;");
    dlg->resize(660, 540);
    auto* lay = new QVBoxLayout(dlg); lay->setSpacing(10);

    auto* hdr = new QLabel(QString::fromUtf8("📋  ") + currentTitle, dlg);
    hdr->setStyleSheet("color:white;font-size:15px;font-weight:bold;");
    lay->addWidget(hdr);

    auto* list = new QListWidget(dlg);
    list->setStyleSheet(
        "QListWidget{background:#111;color:white;border:1px solid #2a2a2a;border-radius:4px;}"
        "QListWidget::item{padding:10px;border-bottom:1px solid #1e1e1e;}"
        "QListWidget::item:selected{background:#1e5c28;}");
    lay->addWidget(list, 1);

    auto fillList = [this, list](){
        list->clear();
        const auto anns = annotManager->getAllAnnotations(currentTitle);
        bool hasAny = false;
        for (const Annotation& a : anns){
            if (a.type == Annotation::Bookmark) continue;
            hasAny = true;
            QString body = a.selectedText.left(90);
            if (!a.noteText.isEmpty())
                body += (body.isEmpty() ? "" : "\n     ↳ ") + a.noteText.left(70);
            auto* item = new QListWidgetItem(
                QString("[Pág. %1]  📝\n%2").arg(a.pageNumber+1).arg(body));
            item->setData(Qt::UserRole,   a.id);
            item->setData(Qt::UserRole+1, a.pageNumber);
            item->setData(Qt::UserRole+2, a.noteText);
            item->setForeground(QColor("#ccc"));
            list->addItem(item);
        }
        if (!hasAny){
            auto* e = new QListWidgetItem(
                "   Nenhuma anotação ainda.\n"
                "   Selecione texto na página ou use o ✏ na topbar.");
            e->setForeground(QColor("#666"));
            list->addItem(e);
        }
    };
    fillList();

    auto* btnRow = new QHBoxLayout();

    auto* goBtn = new QPushButton(QString::fromUtf8("↗ Ir para Página"), dlg);
    goBtn->setStyleSheet("QPushButton{background:#1e6432;color:white;padding:8px;border-radius:6px;}");
    connect(goBtn, &QPushButton::clicked, dlg, [this, list, dlg](){
        if (!list->currentItem() || list->count() == 0) return;
        const int page = list->currentItem()->data(Qt::UserRole+1).toInt();
        dlg->accept();
        currentPage = page;
        windowStart = qMax(0, page - 1);
        renderVisiblePages();
        QTimer::singleShot(80, this, [this](){
            scrollArea->verticalScrollBar()->setValue(
                scrollPositionForPage(currentPage - windowStart));
        });
    });

    auto* editBtn = new QPushButton(QString::fromUtf8("✏ Editar"), dlg);
    editBtn->setStyleSheet("QPushButton{background:#1e3a6a;color:white;padding:8px;border-radius:6px;}");
    connect(editBtn, &QPushButton::clicked, dlg, [this, list, &fillList](){
        if (!list->currentItem() || list->count() == 0) return;
        const int     id   = list->currentItem()->data(Qt::UserRole).toInt();
        const int     page = list->currentItem()->data(Qt::UserRole+1).toInt();
        const QString note = list->currentItem()->data(Qt::UserRole+2).toString();
        openNoteDialog(page, QString(), QList<QRectF>(), id, note);
        fillList();
    });

    auto* delBtn = new QPushButton(QString::fromUtf8("🗑 Excluir"), dlg);
    delBtn->setStyleSheet("QPushButton{background:#7a1010;color:white;padding:8px;border-radius:6px;}");
    connect(delBtn, &QPushButton::clicked, dlg, [this, list, &fillList](){
        if (!list->currentItem() || list->count() == 0) return;
        const int id   = list->currentItem()->data(Qt::UserRole).toInt();
        const int page = list->currentItem()->data(Qt::UserRole+1).toInt();
        annotManager->removeAnnotation(id);
        reloadPageHighlights(page);
        fillList();
    });

    auto* closeBtn = new QPushButton("Fechar", dlg);
    closeBtn->setStyleSheet("QPushButton{background:#333;color:white;padding:8px;border-radius:6px;}");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    btnRow->addWidget(goBtn);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(delBtn);
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    lay->addLayout(btnRow);
    dlg->exec();
    dlg->deleteLater();
}

// ── Janela Deslizante ─────────────────────────────────────────────────────────
void ReaderScreen::expandWindowForward(){
    if (windowStart + MAX_LOADED >= totalPages) return;
    const int saved = scrollArea->verticalScrollBar()->value();
    const int remH  = pageWidgets.isEmpty() ? 0 : pageWidgets.first()->height() + 10;
    windowStart++;
    renderVisiblePages();
    QTimer::singleShot(0, this, [this, saved, remH](){
        scrollArea->verticalScrollBar()->setValue(qMax(0, saved - remH));
    });
}

void ReaderScreen::expandWindowBackward(){
    if (windowStart <= 0) return;
    const int saved = scrollArea->verticalScrollBar()->value();
    windowStart--;
    renderVisiblePages();
    QTimer::singleShot(0, this, [this, saved](){
        const int addH = pageWidgets.isEmpty() ? 0 : pageWidgets.first()->height() + 10;
        scrollArea->verticalScrollBar()->setValue(saved + addH);
    });
}

void ReaderScreen::onScrollDebounced(){
    if (!bookOpen || pageWidgets.isEmpty()) return;
    const int scrollY = scrollArea->verticalScrollBar()->value();
    int cumY = 0, newPage = windowStart;
    for (int i = 0; i < pageWidgets.size(); i++){
        const int h = pageWidgets[i]->height() + 10;
        if (scrollY < cumY + h){ newPage = windowStart + i; break; }
        cumY += h; newPage = windowStart + i;
    }
    if (newPage != currentPage){
        currentPage = newPage;
        updatePageInfo();
        progressManager->saveProgress(currentTitle, currentPage);
    }
    if ((windowStart + MAX_LOADED - 1) - currentPage <= 0 && windowStart + MAX_LOADED < totalPages)
        expandWindowForward();
    else if (currentPage - windowStart <= 0 && windowStart > 0)
        expandWindowBackward();
}

int ReaderScreen::scrollPositionForPage(int localIndex) const {
    int y = 0;
    for (int i = 0; i < localIndex && i < pageWidgets.size(); i++)
        y += pageWidgets[i]->height() + 10;
    return y;
}

void ReaderScreen::updatePageInfo(){
    if (!bookOpen || !pageInfoLabel) return;
    const float pct = totalPages > 1
        ? float(currentPage) / float(totalPages-1) * 100.f
        : 100.f;
    pageInfoLabel->setText(
        QString("%1/%2  (%3%)").arg(currentPage+1).arg(totalPages).arg(int(pct)));
}

// ── Tema ──────────────────────────────────────────────────────────────────────
void ReaderScreen::setMenuColor(const QColor& c){ TopBarHelper::setColor(topBar, c); }
void ReaderScreen::applyNightMode(bool e){
    const QString bg = e ? "#080808" : "#1a1a1a";
    pagesContainer->setStyleSheet(QString("background:%1;").arg(bg));
    scrollArea->setStyleSheet(QString("QScrollArea{border:none;background:%1;}").arg(bg));
}
void ReaderScreen::setAmberIntensity(int v){
    amberIntensity = v;
    for (auto* pw : pageWidgets) pw->setAmberIntensity(v);
}
void ReaderScreen::setSepiaEnabled(bool e){
    sepiaEnabled = e;
    for (auto* pw : pageWidgets) pw->setSepiaEnabled(e);
}
void ReaderScreen::setAppBgColor(const QColor& c){
    pagesContainer->setStyleSheet(QString("background:%1;").arg(c.name()));
    scrollArea->setStyleSheet(
        QString("QScrollArea{border:none;background:%1;}").arg(c.name()));
}

void ReaderScreen::onZoomIn() { if (zoomFactor < 3.f){ zoomFactor += 0.25f; renderVisiblePages(); } }
void ReaderScreen::onZoomOut(){ if (zoomFactor > 0.5f){ zoomFactor -= 0.25f; renderVisiblePages(); } }

void ReaderScreen::onPreviousPage(){
    if (!bookOpen || currentPage <= 0) return;
    currentPage--;
    if (currentPage < windowStart) expandWindowBackward();
    scrollArea->verticalScrollBar()->setValue(scrollPositionForPage(currentPage - windowStart));
}
void ReaderScreen::onNextPage(){
    if (!bookOpen || currentPage >= totalPages-1) return;
    currentPage++;
    if (currentPage >= windowStart + MAX_LOADED) expandWindowForward();
    scrollArea->verticalScrollBar()->setValue(scrollPositionForPage(currentPage - windowStart));
}

void ReaderScreen::keyPressEvent(QKeyEvent* e){
    if (!bookOpen){ QWidget::keyPressEvent(e); return; }
    switch (e->key()){
        case Qt::Key_Down: case Qt::Key_PageDown: case Qt::Key_Space: onNextPage();     break;
        case Qt::Key_Up:   case Qt::Key_PageUp:                       onPreviousPage(); break;
        case Qt::Key_Plus: case Qt::Key_Equal:                        onZoomIn();       break;
        case Qt::Key_Minus:                                           onZoomOut();      break;
        case Qt::Key_Escape: emit backClicked(); break;
        default: QWidget::keyPressEvent(e);
    }
}

void ReaderScreen::resizeEvent(QResizeEvent* e){
    QWidget::resizeEvent(e);
    if (bookOpen) renderVisiblePages();
}
