#include "readerscreen.h"
#include "topbar_helper.h"
#include "../engine/pdfrenderer.h"
#include "../engine/pdfloader.h"          // para getContext() / getDocument()
#include "../engine/textextractor.h"      // TextExtractor::extractWords / pageSize
#include "../engine/pdfcache.h"
#include "../storage/progressmanager.h"
#include "../storage/annotationmanager.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QFileInfo>
#include <QColorDialog>
#include <QDialog>
#include <QTextEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollBar>
#include <QTimer>
#include <QDebug>

// ═══════════════════════════════════════════════════════════════════════════════
//  Helpers para acessar PDFLoader a partir do PDFRenderer.
//  PDFRenderer tem: private unique_ptr<PDFLoader> loader;
//  Precisamos de ctx e doc para o TextExtractor.
//  Solução: criar uma subclasse de PDFRenderer que expõe o loader,
//  OU acessar por um getter que adicionaremos ao PDFRenderer.
//
//  Como não queremos modificar PDFRenderer, usamos uma abordagem diferente:
//  criamos um PDFLoader separado apenas para extração de texto.
//  O PDFRenderer cuida do render, o TextLoader cuida da extração.
// ═══════════════════════════════════════════════════════════════════════════════

ReaderScreen::ReaderScreen(QWidget* parent)
    : QWidget(parent)
    , renderer(std::make_unique<PDFRenderer>())
    , progressManager(std::make_unique<ProgressManager>())
    , annotManager(std::make_unique<AnnotationManager>())
{
    setupUI();
    setFocusPolicy(Qt::StrongFocus);
}

ReaderScreen::~ReaderScreen() { closeBook(); }

// ─── Setup ───────────────────────────────────────────────────────────────────

void ReaderScreen::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setupTopBar();
    setupScrollArea();
    setupBottomBar();
    setupAnnotationPanel();
}

void ReaderScreen::setupTopBar()
{
    backButton = new QPushButton("←");
    backButton->setFixedSize(40, 40);
    backButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:22px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(backButton, &QPushButton::clicked, this, &ReaderScreen::backClicked);

    annotBtn = new QPushButton("🖊");
    annotBtn->setFixedSize(36, 36);
    annotBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:18px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:18px;}");
    connect(annotBtn, &QPushButton::clicked, this, &ReaderScreen::onToggleAnnotationPanel);

    topBar = TopBarHelper::create(this, backButton, annotBtn);

    pageInfoLabel = new QLabel("", topBar);
    pageInfoLabel->setStyleSheet(
        "color:rgba(255,255,255,0.85);font-size:11px;min-width:80px;");
    pageInfoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto* lay = qobject_cast<QHBoxLayout*>(topBar->layout());
    if (lay) lay->insertWidget(lay->count() - 1, pageInfoLabel);

    mainLayout->addWidget(topBar);
}

void ReaderScreen::setupScrollArea()
{
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet("QScrollArea{border:none;background:#1a1a1a;}");

    pagesContainer = new QWidget();
    pagesContainer->setStyleSheet("background:#1a1a1a;");
    pagesLayout = new QVBoxLayout(pagesContainer);
    pagesLayout->setContentsMargins(0, 8, 0, 8);
    pagesLayout->setSpacing(8);
    pagesLayout->setAlignment(Qt::AlignHCenter);

    scrollArea->setWidget(pagesContainer);
    scrollArea->viewport()->installEventFilter(this);

    scrollDebounce = new QTimer(this);
    scrollDebounce->setSingleShot(true);
    scrollDebounce->setInterval(80);
    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int) { scrollDebounce->start(); });
    connect(scrollDebounce, &QTimer::timeout,
            this, &ReaderScreen::onScrollDebounced);

    mainLayout->addWidget(scrollArea, 1);
}

void ReaderScreen::setupBottomBar()
{
    bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(44);
    bottomBar->setStyleSheet("background:#111;border-top:1px solid #333;");

    QHBoxLayout* lay = new QHBoxLayout(bottomBar);
    lay->setContentsMargins(8, 4, 8, 4);
    lay->setSpacing(8);

    zoomOutBtn = new QPushButton("−", bottomBar);
    zoomOutBtn->setFixedSize(36, 36);
    zoomOutBtn->setStyleSheet(
        "QPushButton{background:#333;color:white;font-size:20px;border-radius:18px;}"
        "QPushButton:pressed{background:#555;}");
    connect(zoomOutBtn, &QPushButton::clicked, this, &ReaderScreen::onZoomOut);

    progressLabel = new QLabel("0%", bottomBar);
    progressLabel->setStyleSheet("color:#aaa;font-size:12px;");
    progressLabel->setAlignment(Qt::AlignCenter);

    zoomInBtn = new QPushButton("+", bottomBar);
    zoomInBtn->setFixedSize(36, 36);
    zoomInBtn->setStyleSheet(zoomOutBtn->styleSheet());
    connect(zoomInBtn, &QPushButton::clicked, this, &ReaderScreen::onZoomIn);

    bookmarkBtn = new QPushButton("🔖", bottomBar);
    bookmarkBtn->setFixedSize(36, 36);
    bookmarkBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:18px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.15);border-radius:18px;}");
    connect(bookmarkBtn, &QPushButton::clicked, this, &ReaderScreen::onToggleBookmark);

    QPushButton* listBtn = new QPushButton("📋", bottomBar);
    listBtn->setFixedSize(36, 36);
    listBtn->setStyleSheet(bookmarkBtn->styleSheet());
    connect(listBtn, &QPushButton::clicked, this, &ReaderScreen::onShowAnnotationsList);

    lay->addWidget(zoomOutBtn);
    lay->addWidget(progressLabel, 1);
    lay->addWidget(zoomInBtn);
    lay->addSpacing(4);
    lay->addWidget(bookmarkBtn);
    lay->addWidget(listBtn);

    mainLayout->addWidget(bottomBar);
}

void ReaderScreen::setupAnnotationPanel()
{
    annotationPanel = new QWidget(this);
    annotationPanel->setFixedWidth(290);
    annotationPanel->setStyleSheet("background:#1a1a1a;border-left:2px solid #333;");
    annotationPanel->hide();

    QVBoxLayout* pl = new QVBoxLayout(annotationPanel);
    pl->setContentsMargins(12, 12, 12, 12);
    pl->setSpacing(8);

    QLabel* title = new QLabel("✏️  Grifar / Anotar", annotationPanel);
    title->setStyleSheet("color:white;font-size:15px;font-weight:bold;");
    pl->addWidget(title);

    selTextPreview = new QLabel("", annotationPanel);
    selTextPreview->setStyleSheet(
        "color:#ddd;font-size:11px;background:#252525;border-radius:4px;"
        "padding:6px;border-left:3px solid #FFD700;");
    selTextPreview->setWordWrap(true);
    selTextPreview->setMaximumHeight(90);
    pl->addWidget(selTextPreview);

    // Linha cor
    QHBoxLayout* colorRow = new QHBoxLayout();
    QLabel* colorLbl = new QLabel("Cor:", annotationPanel);
    colorLbl->setStyleSheet("color:#aaa;font-size:12px;");
    hlColorPreview = new QLabel(annotationPanel);
    hlColorPreview->setFixedSize(28, 28);
    hlColorPreview->setStyleSheet(
        QString("background:%1;border-radius:4px;border:2px solid #888;")
        .arg(currentHighlightColor.name()));

    QPushButton* pickColorBtn = new QPushButton("🎨", annotationPanel);
    pickColorBtn->setFixedSize(34, 34);
    pickColorBtn->setStyleSheet(
        "QPushButton{background:#333;color:white;font-size:15px;border-radius:6px;}"
        "QPushButton:pressed{background:#555;}");
    connect(pickColorBtn, &QPushButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(currentHighlightColor, this, "Cor do Grifador");
        if (c.isValid()) {
            currentHighlightColor = c;
            hlColorPreview->setStyleSheet(
                QString("background:%1;border-radius:4px;border:2px solid #888;")
                .arg(c.name()));
        }
    });
    colorRow->addWidget(colorLbl);
    colorRow->addWidget(hlColorPreview);
    colorRow->addWidget(pickColorBtn);
    colorRow->addStretch();
    pl->addLayout(colorRow);

    // Cores rápidas
    QHBoxLayout* qcRow = new QHBoxLayout();
    qcRow->setSpacing(6);
    const QList<QColor> qcs = {
        QColor(255,235,59), QColor(76,175,80),  QColor(33,150,243),
        QColor(244,67,54),  QColor(255,152,0),  QColor(156,39,176)
    };
    for (const QColor& cc : qcs) {
        QPushButton* cb = new QPushButton(annotationPanel);
        cb->setFixedSize(26, 26);
        cb->setStyleSheet(
            QString("QPushButton{background:%1;border-radius:13px;border:2px solid #555;}"
                    "QPushButton:pressed{border-color:white;}").arg(cc.name()));
        connect(cb, &QPushButton::clicked, this, [this, cc]() {
            currentHighlightColor = cc;
            hlColorPreview->setStyleSheet(
                QString("background:%1;border-radius:4px;border:2px solid #888;")
                .arg(cc.name()));
        });
        qcRow->addWidget(cb);
    }
    pl->addLayout(qcRow);

    // Grifo simples
    QPushButton* highlightBtn = new QPushButton("🖍  Grifo Simples", annotationPanel);
    highlightBtn->setStyleSheet(
        "QPushButton{background:#2a5020;color:white;padding:9px;border-radius:6px;font-size:13px;}"
        "QPushButton:pressed{background:#3a7030;}");
    connect(highlightBtn, &QPushButton::clicked, this, [this]() {
        if (pendingSelText.isEmpty() || pendingSelPageRects.isEmpty()) return;
        annotManager->addHighlight(currentTitle, pendingSelPage,
                                    pendingSelText, currentHighlightColor,
                                    pendingSelPageRects);
        reloadPageHighlights(pendingSelPage);
        closeAnnotationPanel();
    });
    pl->addWidget(highlightBtn);

    // Grifo + nota
    QPushButton* noteBtn = new QPushButton("📝  Grifo + Anotação", annotationPanel);
    noteBtn->setStyleSheet(
        "QPushButton{background:#1e3a6a;color:white;padding:9px;border-radius:6px;font-size:13px;}"
        "QPushButton:pressed{background:#2e4a8a;}");
    connect(noteBtn, &QPushButton::clicked, this, [this]() {
        if (pendingSelText.isEmpty()) return;
        openNoteDialog(-1, pendingSelText, pendingSelPageRects, pendingSelPage, "");
    });
    pl->addWidget(noteBtn);

    // Fechar
    QPushButton* closeBtn = new QPushButton("✕  Fechar", annotationPanel);
    closeBtn->setStyleSheet(
        "QPushButton{background:#2a2a2a;color:#aaa;padding:7px;border-radius:6px;}"
        "QPushButton:pressed{background:#444;}");
    connect(closeBtn, &QPushButton::clicked, this, [this]() { closeAnnotationPanel(); });
    pl->addWidget(closeBtn);
}

// ─── Abertura / Fechamento ───────────────────────────────────────────────────

void ReaderScreen::openBook(const QString& filePath)
{
    closeBook();
    currentFilePath = filePath;
    currentTitle    = QFileInfo(filePath).completeBaseName();

    if (!renderer->openPDF(filePath)) return;

    bookOpen    = true;
    totalPages  = renderer->getPageCount();
    currentPage = progressManager->getLastPage(currentTitle);
    if (currentPage < 0 || currentPage >= totalPages) currentPage = 0;

    topBar->show(); bottomBar->show();
    topBarVisible = true;
    windowStart   = qMax(0, currentPage - 1);

    // FIX páginas pequenas: adia renderização até o layout ter processado
    QTimer::singleShot(0, this, [this]() {
        renderVisiblePages();
        QTimer::singleShot(60, this, [this]() {
            scrollArea->verticalScrollBar()->setValue(
                scrollPositionForPage(currentPage - windowStart));
        });
    });
    setFocus();
}

void ReaderScreen::closeBook()
{
    if (!bookOpen) return;
    progressManager->saveProgress(currentTitle, currentPage);
    renderer->closePDF();
    for (auto* w : pageWidgets) { pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();
    bookOpen    = false;
    currentPage = 0;
    totalPages  = 0;
    if (pageInfoLabel)  pageInfoLabel->clear();
    if (progressLabel)  progressLabel->setText("0%");
}

// ─── Renderização ────────────────────────────────────────────────────────────

void ReaderScreen::renderVisiblePages()
{
    if (!bookOpen) return;

    for (auto* w : pageWidgets) { pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();

    // FIX: se viewport ainda não tem tamanho, adia
    int availW = scrollArea->viewport()->width();
    if (availW < 50) {
        QTimer::singleShot(20, this, &ReaderScreen::renderVisiblePages);
        return;
    }

    const int end = qMin(windowStart + MAX_LOADED, totalPages);

    // Criamos um PDFLoader auxiliar apenas para extração de texto/tamanho de página.
    // O PDFRenderer renderiza, o PDFLoader auxiliar fornece ctx+doc para o TextExtractor.
    auto textLoader = std::make_unique<PDFLoader>();
    const bool textLoaderOk = textLoader->openDocument(currentFilePath);

    for (int p = windowStart; p < end; p++) {
        PageWidget* pw = new PageWidget(pagesContainer);
        pw->setAmberIntensity(amberIntensity);
        pw->setSepiaEnabled(sepiaEnabled);

        const int renderW = qRound(availW * zoomFactor);
        const int renderH = qRound(renderW * 1.414f);

        QPixmap pix = renderer->renderPage(p, renderW, renderH);

        if (!pix.isNull()) {
            const QPixmap scaled = pix.scaled(
                renderW, renderH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            pw->setPagePixmap(scaled);
            pw->setFixedSize(scaled.size());

            // Escala para converter coords de página → widget
            if (textLoaderOk) {
                const QSizeF ps = TextExtractor::pageSize(
                    textLoader->getContext(), textLoader->getDocument(), p);
                if (ps.width() > 0 && ps.height() > 0) {
                    const float sx = float(scaled.width())  / float(ps.width());
                    const float sy = float(scaled.height()) / float(ps.height());
                    pw->setRenderScale(sx, sy);
                    pw->setPageWords(TextExtractor::extractWords(
                        textLoader->getContext(), textLoader->getDocument(), p));
                }
            }
        } else {
            pw->setText(QString("Página %1").arg(p + 1));
            pw->setFixedSize(renderW, renderH);
        }

        loadHighlightsForPage(pw, p);

        const int pageIndex = p;
        connect(pw, &PageWidget::selectionFinished, this,
            [this, pageIndex](const QString& text, const QList<QRectF>& pageRects) {
                onSelectionFinished(text, pageRects, pageIndex);
            });

        pagesLayout->addWidget(pw, 0, Qt::AlignHCenter);
        pageWidgets.append(pw);
    }

    updatePageInfo();
    updateProgressBar();
}

void ReaderScreen::loadHighlightsForPage(PageWidget* pw, int pageIndex)
{
    const auto anns = annotManager->getAnnotationsForPage(currentTitle, pageIndex);
    QList<PageHighlight> hls;
    for (const auto& a : anns) {
        if ((a.type == Annotation::Highlight || a.type == Annotation::Note)
            && !a.rects.isEmpty()) {
            PageHighlight hl;
            hl.pageRects    = a.rects;           // coords de página (PDF pts)
            hl.color        = a.highlightColor;
            hl.annotationId = a.id;
            hls.append(hl);
        }
    }
    pw->setHighlights(hls);
}

void ReaderScreen::reloadPageHighlights(int pageIndex)
{
    const int local = pageIndex - windowStart;
    if (local >= 0 && local < pageWidgets.size())
        loadHighlightsForPage(pageWidgets[local], pageIndex);
}

// ─── Painel de anotação ──────────────────────────────────────────────────────

void ReaderScreen::onSelectionFinished(const QString& text,
                                        const QList<QRectF>& pageRects,
                                        int pageIndex)
{
    if (text.trimmed().isEmpty()) return;
    pendingSelText      = text.trimmed();
    pendingSelPageRects = pageRects;
    pendingSelPage      = pageIndex;

    QString preview = pendingSelText.left(130);
    if (pendingSelText.length() > 130) preview += "…";
    selTextPreview->setText(preview);
    showAnnotationPanel();
}

void ReaderScreen::showAnnotationPanel()
{
    annotPanelVisible = true;
    annotationPanel->setParent(this);
    annotationPanel->setGeometry(
        width() - 290,
        topBar->isVisible()    ? topBar->height()    : 0,
        290,
        height()
            - (topBar->isVisible()    ? topBar->height()    : 0)
            - (bottomBar->isVisible() ? bottomBar->height() : 0));
    annotationPanel->raise();
    annotationPanel->show();
}

void ReaderScreen::closeAnnotationPanel()
{
    annotationPanel->hide();
    annotPanelVisible = false;
    pendingSelText.clear();
    pendingSelPageRects.clear();
}

void ReaderScreen::onToggleAnnotationPanel()
{
    if (annotPanelVisible) {
        closeAnnotationPanel();
    } else {
        selTextPreview->setText(
            "Arraste o dedo sobre o texto para selecioná-lo,\n"
            "depois toque em Grifo Simples ou Grifo + Anotação.");
        pendingSelText.clear();
        showAnnotationPanel();
    }
}

// ─── Dialog nota (criar ou editar) ──────────────────────────────────────────

void ReaderScreen::openNoteDialog(int annotId,
                                   const QString&       selText,
                                   const QList<QRectF>& pageRects,
                                   int                  pageIndex,
                                   const QString&       existingNote)
{
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle(annotId < 0 ? "Nova Anotação" : "Editar Anotação");
    dlg->setStyleSheet("background:#1a1a1a;color:white;");
    dlg->resize(520, 380);

    QVBoxLayout* dl = new QVBoxLayout(dlg);

    if (!selText.isEmpty()) {
        QLabel* citLbl = new QLabel("Citação:", dlg);
        citLbl->setStyleSheet("color:#aaa;font-size:12px;");
        dl->addWidget(citLbl);
        QLabel* cit = new QLabel(selText.left(200) + (selText.length()>200?"…":""), dlg);
        cit->setStyleSheet(
            QString("color:#eee;background:%1;padding:8px;border-radius:4px;font-size:12px;")
            .arg(currentHighlightColor.name()));
        cit->setWordWrap(true);
        dl->addWidget(cit);
    }

    QLabel* noteLbl = new QLabel("Anotação:", dlg);
    noteLbl->setStyleSheet("color:#aaa;font-size:12px;");
    dl->addWidget(noteLbl);

    QTextEdit* noteEdit = new QTextEdit(dlg);
    noteEdit->setStyleSheet(
        "QTextEdit{background:#252525;color:white;border:1px solid #444;"
        "border-radius:4px;padding:6px;font-size:13px;}");
    noteEdit->setPlaceholderText("Escreva sua anotação aqui...");
    noteEdit->setText(existingNote);
    dl->addWidget(noteEdit, 1);

    QHBoxLayout* btnRow = new QHBoxLayout();
    QPushButton* ok = new QPushButton(annotId < 0 ? "✓ Salvar Grifo" : "✓ Salvar", dlg);
    ok->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:10px;border-radius:6px;}"
        "QPushButton:pressed{background:#2a8040;}");
    QPushButton* cancel = new QPushButton("Cancelar", dlg);
    cancel->setStyleSheet(
        "QPushButton{background:#333;color:white;padding:10px;border-radius:6px;}"
        "QPushButton:pressed{background:#555;}");
    connect(ok,     &QPushButton::clicked, dlg, &QDialog::accept);
    connect(cancel, &QPushButton::clicked, dlg, &QDialog::reject);
    btnRow->addWidget(cancel);
    btnRow->addWidget(ok);
    dl->addLayout(btnRow);

    if (dlg->exec() == QDialog::Accepted) {
        const QString note = noteEdit->toPlainText().trimmed();
        if (annotId < 0) {
            annotManager->addNote(currentTitle, pageIndex,
                                   note, selText, currentHighlightColor, pageRects);
        } else {
            annotManager->updateNote(annotId, note);
        }
        reloadPageHighlights(pageIndex);
    }
    dlg->deleteLater();
    closeAnnotationPanel();
}

// ─── Lista de anotações ──────────────────────────────────────────────────────

void ReaderScreen::onShowAnnotationsList()
{
    if (!bookOpen) return;

    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Anotações — " + currentTitle);
    dlg->setStyleSheet("background:#1a1a1a;color:white;");
    dlg->resize(640, 520);

    QVBoxLayout* lay = new QVBoxLayout(dlg);
    QLabel* hdr = new QLabel("📋  Todas as Anotações", dlg);
    hdr->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
    lay->addWidget(hdr);

    QListWidget* list = new QListWidget(dlg);
    list->setStyleSheet(
        "QListWidget{background:#111;color:white;border:none;}"
        "QListWidget::item{padding:10px;border-bottom:1px solid #222;}"
        "QListWidget::item:selected{background:#1e6432;}");

    auto reloadList = [&]() {
        list->clear();
        for (const auto& a : annotManager->getAllAnnotations(currentTitle)) {
            const QString typeStr =
                a.type == Annotation::Highlight ? "🖍 Grifo"
              : a.type == Annotation::Note      ? "📝 Nota"
              : "🔖 Marcador";
            QString body = a.selectedText;
            if (!a.noteText.isEmpty()) body += "\n   ↳ " + a.noteText;
            auto* item = new QListWidgetItem(
                QString("[P.%1]  %2\n%3").arg(a.pageNumber+1).arg(typeStr).arg(body));
            item->setData(Qt::UserRole,   a.id);
            item->setData(Qt::UserRole+1, a.pageNumber);
            item->setData(Qt::UserRole+2, static_cast<int>(a.type));
            item->setData(Qt::UserRole+3, a.noteText);
            item->setData(Qt::UserRole+4, a.selectedText);
            if (a.type != Annotation::Bookmark)
                item->setForeground(a.highlightColor);
            list->addItem(item);
        }
    };
    reloadList();

    QHBoxLayout* btnRow = new QHBoxLayout();

    // Ir para página
    QPushButton* goBtn = new QPushButton("↗ Ir para Página", dlg);
    goBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#2a8040;}");
    connect(goBtn, &QPushButton::clicked, dlg, [this, list, dlg]() {
        if (!list->currentItem()) return;
        const int page = list->currentItem()->data(Qt::UserRole+1).toInt();
        dlg->accept();
        currentPage = page;
        windowStart = qMax(0, page - 1);
        renderVisiblePages();
        QTimer::singleShot(80, this, [this]() {
            scrollArea->verticalScrollBar()->setValue(
                scrollPositionForPage(currentPage - windowStart));
        });
    });

    // Editar nota
    QPushButton* editBtn = new QPushButton("✎ Editar Nota", dlg);
    editBtn->setStyleSheet(
        "QPushButton{background:#1e3a6a;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#2e4a8a;}");
    connect(editBtn, &QPushButton::clicked, dlg, [this, list, &reloadList]() {
        if (!list->currentItem()) return;
        const int  id       = list->currentItem()->data(Qt::UserRole).toInt();
        const int  page     = list->currentItem()->data(Qt::UserRole+1).toInt();
        const int  type     = list->currentItem()->data(Qt::UserRole+2).toInt();
        const QString existing = list->currentItem()->data(Qt::UserRole+3).toString();
        const QString selTxt   = list->currentItem()->data(Qt::UserRole+4).toString();
        if (type == static_cast<int>(Annotation::Bookmark)) return;

        QDialog* ed = new QDialog(this);
        ed->setWindowTitle("Editar Anotação");
        ed->setStyleSheet("background:#1a1a1a;color:white;");
        ed->resize(480, 300);
        QVBoxLayout* el = new QVBoxLayout(ed);
        if (!selTxt.isEmpty()) {
            QLabel* cit = new QLabel(selTxt.left(150), ed);
            cit->setStyleSheet(
                "color:#ccc;background:#252525;padding:6px;border-radius:4px;font-size:11px;");
            cit->setWordWrap(true);
            el->addWidget(cit);
        }
        QTextEdit* te = new QTextEdit(ed);
        te->setStyleSheet(
            "QTextEdit{background:#252525;color:white;border:1px solid #444;"
            "border-radius:4px;padding:6px;font-size:13px;}");
        te->setText(existing);
        el->addWidget(te, 1);
        QHBoxLayout* eb = new QHBoxLayout();
        QPushButton* sv = new QPushButton("✓ Salvar", ed);
        sv->setStyleSheet(
            "QPushButton{background:#1e6432;color:white;padding:9px;border-radius:6px;}");
        QPushButton* cn = new QPushButton("Cancelar", ed);
        cn->setStyleSheet(
            "QPushButton{background:#333;color:white;padding:9px;border-radius:6px;}");
        connect(sv, &QPushButton::clicked, ed, &QDialog::accept);
        connect(cn, &QPushButton::clicked, ed, &QDialog::reject);
        eb->addWidget(cn); eb->addWidget(sv);
        el->addLayout(eb);
        if (ed->exec() == QDialog::Accepted) {
            annotManager->updateNote(id, te->toPlainText().trimmed());
            reloadPageHighlights(page);
            reloadList();
        }
        ed->deleteLater();
    });

    // Excluir
    QPushButton* delBtn = new QPushButton("🗑 Excluir", dlg);
    delBtn->setStyleSheet(
        "QPushButton{background:#7a1010;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#a01010;}");
    connect(delBtn, &QPushButton::clicked, dlg, [this, list, &reloadList]() {
        if (!list->currentItem()) return;
        const int id   = list->currentItem()->data(Qt::UserRole).toInt();
        const int page = list->currentItem()->data(Qt::UserRole+1).toInt();
        annotManager->removeAnnotation(id);
        reloadPageHighlights(page);
        reloadList();
    });

    QPushButton* closeBtn = new QPushButton("Fechar", dlg);
    closeBtn->setStyleSheet(
        "QPushButton{background:#333;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#555;}");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    btnRow->addWidget(goBtn);
    btnRow->addWidget(editBtn);
    btnRow->addWidget(delBtn);
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    lay->addWidget(list, 1);
    lay->addLayout(btnRow);

    dlg->exec();
    dlg->deleteLater();
}

// ─── Expansão de janela deslizante ───────────────────────────────────────────

void ReaderScreen::expandWindowForward()
{
    if (windowStart + MAX_LOADED >= totalPages) return;
    const int saved    = scrollArea->verticalScrollBar()->value();
    const int removedH = pageWidgets.isEmpty() ? 0 : pageWidgets.first()->height() + 8;
    windowStart++;
    renderVisiblePages();
    QTimer::singleShot(0, this, [this, saved, removedH]() {
        scrollArea->verticalScrollBar()->setValue(qMax(0, saved - removedH));
    });
}

void ReaderScreen::expandWindowBackward()
{
    if (windowStart <= 0) return;
    const int saved = scrollArea->verticalScrollBar()->value();
    windowStart--;
    renderVisiblePages();
    QTimer::singleShot(0, this, [this, saved]() {
        const int addedH = pageWidgets.isEmpty() ? 0 : pageWidgets.first()->height() + 8;
        scrollArea->verticalScrollBar()->setValue(saved + addedH);
    });
}

// ─── Scroll debounced ────────────────────────────────────────────────────────

void ReaderScreen::onScrollDebounced()
{
    if (!bookOpen || pageWidgets.isEmpty()) return;
    const int scrollY = scrollArea->verticalScrollBar()->value();
    int cumY = 0, newPage = windowStart;
    for (int i = 0; i < pageWidgets.size(); i++) {
        const int h = pageWidgets[i]->height() + 8;
        if (scrollY < cumY + h) { newPage = windowStart + i; break; }
        cumY += h;
        newPage = windowStart + i;
    }
    if (newPage != currentPage) {
        currentPage = newPage;
        updatePageInfo();
        updateProgressBar();
        progressManager->saveProgress(currentTitle, currentPage);
        bookmarkBtn->setStyleSheet(
            annotManager->hasBookmark(currentTitle, currentPage)
            ? "QPushButton{background:rgba(255,200,0,0.3);color:#FFD700;"
              "font-size:18px;border:none;border-radius:18px;}"
            : "QPushButton{background:transparent;color:white;"
              "font-size:18px;border:none;}");
    }
    const int distToEnd   = (windowStart + MAX_LOADED - 1) - currentPage;
    const int distToStart = currentPage - windowStart;
    if (distToEnd   <= 0 && windowStart + MAX_LOADED < totalPages) expandWindowForward();
    else if (distToStart <= 0 && windowStart > 0)                  expandWindowBackward();
}

int ReaderScreen::scrollPositionForPage(int localIndex) const
{
    int y = 0;
    for (int i = 0; i < localIndex && i < pageWidgets.size(); i++)
        y += pageWidgets[i]->height() + 8;
    return y;
}

// ─── Info ────────────────────────────────────────────────────────────────────

void ReaderScreen::updatePageInfo()
{
    if (!bookOpen || !pageInfoLabel) return;
    const float pct = totalPages > 1
        ? float(currentPage) / float(totalPages - 1) * 100.f
        : 100.f;
    pageInfoLabel->setText(
        QString("%1/%2 (%3%)").arg(currentPage+1).arg(totalPages).arg(int(pct)));
}

void ReaderScreen::updateProgressBar()
{
    if (!bookOpen || !progressLabel) return;
    const float pct = totalPages > 1
        ? float(currentPage) / float(totalPages - 1) * 100.f
        : 100.f;
    progressLabel->setText(QString("%1%").arg(int(pct)));
}

// ─── Tema ────────────────────────────────────────────────────────────────────

void ReaderScreen::setMenuColor(const QColor& c) { TopBarHelper::setColor(topBar, c); }

void ReaderScreen::applyNightMode(bool e) {
    const QString bg = e ? "#080808" : "#1a1a1a";
    pagesContainer->setStyleSheet(QString("background:%1;").arg(bg));
    scrollArea->setStyleSheet(QString("QScrollArea{border:none;background:%1;}").arg(bg));
}

void ReaderScreen::setAmberIntensity(int v) {
    amberIntensity = v;
    for (auto* pw : pageWidgets) pw->setAmberIntensity(v);
}

void ReaderScreen::setSepiaEnabled(bool e) {
    sepiaEnabled = e;
    for (auto* pw : pageWidgets) pw->setSepiaEnabled(e);
}

// ─── Zoom ────────────────────────────────────────────────────────────────────

void ReaderScreen::onZoomIn()
{
    if (zoomFactor >= 3.0f) return;
    zoomFactor += 0.25f;
    renderVisiblePages();
}

void ReaderScreen::onZoomOut()
{
    if (zoomFactor <= 0.5f) return;
    zoomFactor -= 0.25f;
    renderVisiblePages();
}

// ─── Bookmark ────────────────────────────────────────────────────────────────

void ReaderScreen::onToggleBookmark()
{
    if (!bookOpen) return;
    if (annotManager->hasBookmark(currentTitle, currentPage)) {
        for (const auto& a : annotManager->getAnnotationsForPage(currentTitle, currentPage))
            if (a.type == Annotation::Bookmark) annotManager->removeAnnotation(a.id);
        bookmarkBtn->setStyleSheet(
            "QPushButton{background:transparent;color:white;font-size:18px;border:none;}");
    } else {
        annotManager->addBookmark(currentTitle, currentPage);
        bookmarkBtn->setStyleSheet(
            "QPushButton{background:rgba(255,200,0,0.3);color:#FFD700;"
            "font-size:18px;border:none;border-radius:18px;}");
    }
}

// ─── eventFilter (tap toggle barras) ────────────────────────────────────────

bool ReaderScreen::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == scrollArea->viewport()) {
        if (ev->type() == QEvent::MouseButtonPress) {
            tapStartPos = static_cast<QMouseEvent*>(ev)->pos();
            tapMoved    = false;
        } else if (ev->type() == QEvent::MouseMove) {
            if ((static_cast<QMouseEvent*>(ev)->pos() - tapStartPos).manhattanLength() > 12)
                tapMoved = true;
        } else if (ev->type() == QEvent::MouseButtonRelease) {
            if (!tapMoved) {
                topBarVisible = !topBarVisible;
                topBar->setVisible(topBarVisible);
                bottomBar->setVisible(topBarVisible);
                if (annotPanelVisible && !topBarVisible) closeAnnotationPanel();
            }
            tapMoved = false;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

// ─── Teclado ─────────────────────────────────────────────────────────────────

void ReaderScreen::onPreviousPage() {
    if (!bookOpen || currentPage <= 0) return;
    currentPage--;
    if (currentPage < windowStart) expandWindowBackward();
    scrollArea->verticalScrollBar()->setValue(
        scrollPositionForPage(currentPage - windowStart));
}

void ReaderScreen::onNextPage() {
    if (!bookOpen || currentPage >= totalPages - 1) return;
    currentPage++;
    if (currentPage >= windowStart + MAX_LOADED) expandWindowForward();
    scrollArea->verticalScrollBar()->setValue(
        scrollPositionForPage(currentPage - windowStart));
}

void ReaderScreen::keyPressEvent(QKeyEvent* e)
{
    if (!bookOpen) { QWidget::keyPressEvent(e); return; }
    switch (e->key()) {
        case Qt::Key_Down: case Qt::Key_PageDown: case Qt::Key_Space: onNextPage(); break;
        case Qt::Key_Up:   case Qt::Key_PageUp:                       onPreviousPage(); break;
        case Qt::Key_Plus: case Qt::Key_Equal: onZoomIn(); break;
        case Qt::Key_Minus:                    onZoomOut(); break;
        case Qt::Key_Escape: emit backClicked(); break;
        default: QWidget::keyPressEvent(e);
    }
}

void ReaderScreen::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    if (bookOpen) renderVisiblePages();
    if (annotPanelVisible) showAnnotationPanel();
}
