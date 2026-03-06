#include "readerscreen.h"
#include "topbar_helper.h"
#include "../engine/pdfrenderer.h"
#include "../engine/pdfloader.h"
#include "../engine/textextractor.h"
#include "../storage/progressmanager.h"
#include "../storage/annotationmanager.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QFileInfo>
#include <QColorDialog>
#include <QDialog>
#include <QTextEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScrollBar>
#include <QTimer>
#include <QDebug>

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

// ── Setup ─────────────────────────────────────────────────────────────────────

void ReaderScreen::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    setupTopBar();
    setupScrollArea();
    setupBottomBar();
    setupAnnotationPanel();
}

void ReaderScreen::setupTopBar()
{
    backButton = new QPushButton("←");
    backButton->setFixedSize(40,40);
    backButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:22px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(backButton, &QPushButton::clicked, this, &ReaderScreen::backClicked);

    annotBtn = new QPushButton(QString::fromUtf8("✏"));
    annotBtn->setFixedSize(36,36);
    annotBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:18px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:18px;}");
    connect(annotBtn, &QPushButton::clicked, this, &ReaderScreen::onToggleAnnotationPanel);

    topBar = TopBarHelper::create(this, backButton, annotBtn);

    pageInfoLabel = new QLabel("", topBar);
    pageInfoLabel->setStyleSheet("color:rgba(255,255,255,0.85);font-size:11px;min-width:90px;");
    pageInfoLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    auto* lay = qobject_cast<QHBoxLayout*>(topBar->layout());
    if (lay) lay->insertWidget(lay->count()-1, pageInfoLabel);

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

void ReaderScreen::setupBottomBar()
{
    bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(50);
    bottomBar->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "stop:0 #1a1a1a,stop:1 #111);border-top:1px solid #2a2a2a;");

    QHBoxLayout* lay = new QHBoxLayout(bottomBar);
    lay->setContentsMargins(12,5,12,5);
    lay->setSpacing(8);

    const QString btnS =
        "QPushButton{background:#2a2a2a;color:white;border-radius:18px;font-size:18px;"
        "border:1px solid #3a3a3a;}"
        "QPushButton:pressed{background:#444;}";

    zoomOutBtn = new QPushButton("−", bottomBar);
    zoomOutBtn->setFixedSize(36,36);
    zoomOutBtn->setStyleSheet(btnS);
    connect(zoomOutBtn, &QPushButton::clicked, this, &ReaderScreen::onZoomOut);

    progressLabel = new QLabel("0%", bottomBar);
    progressLabel->setStyleSheet("color:#aaa;font-size:13px;");
    progressLabel->setAlignment(Qt::AlignCenter);

    zoomInBtn = new QPushButton("+", bottomBar);
    zoomInBtn->setFixedSize(36,36);
    zoomInBtn->setStyleSheet(btnS);
    connect(zoomInBtn, &QPushButton::clicked, this, &ReaderScreen::onZoomIn);

    bookmarkBtn = new QPushButton(QString::fromUtf8("🔖"), bottomBar);
    bookmarkBtn->setFixedSize(36,36);
    bookmarkBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:18px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.15);border-radius:18px;}");
    connect(bookmarkBtn, &QPushButton::clicked, this, &ReaderScreen::onToggleBookmark);

    QPushButton* listBtn = new QPushButton(QString::fromUtf8("📋"), bottomBar);
    listBtn->setFixedSize(36,36);
    listBtn->setStyleSheet(bookmarkBtn->styleSheet());
    connect(listBtn, &QPushButton::clicked, this, &ReaderScreen::onShowAnnotationsList);

    lay->addWidget(zoomOutBtn);
    lay->addWidget(progressLabel,1);
    lay->addWidget(zoomInBtn);
    lay->addSpacing(6);
    lay->addWidget(bookmarkBtn);
    lay->addWidget(listBtn);
    mainLayout->addWidget(bottomBar);
}

void ReaderScreen::setupAnnotationPanel()
{
    annotationPanel = new QWidget(this);
    annotationPanel->setFixedWidth(275);
    annotationPanel->setStyleSheet(
        "background:#1c1c1c;border-left:2px solid #2a6434;");
    annotationPanel->hide();

    QVBoxLayout* pl = new QVBoxLayout(annotationPanel);
    pl->setContentsMargins(14,14,14,14);
    pl->setSpacing(10);

    QLabel* title = new QLabel(QString::fromUtf8("✏  Grifar / Anotar"), annotationPanel);
    title->setStyleSheet("color:white;font-size:14px;font-weight:bold;");
    pl->addWidget(title);

    selTextPreview = new QLabel("", annotationPanel);
    selTextPreview->setStyleSheet(
        "color:#ddd;font-size:11px;background:#252525;border-radius:5px;"
        "padding:8px;border-left:3px solid #FFD700;");
    selTextPreview->setWordWrap(true);
    selTextPreview->setMaximumHeight(75);
    pl->addWidget(selTextPreview);

    // Cores rápidas
    QHBoxLayout* colorRow = new QHBoxLayout();
    QLabel* colorLbl = new QLabel("Cor:", annotationPanel);
    colorLbl->setStyleSheet("color:#aaa;font-size:12px;");
    hlColorPreview = new QLabel(annotationPanel);
    hlColorPreview->setFixedSize(24,24);
    hlColorPreview->setStyleSheet(
        QString("background:%1;border-radius:4px;border:2px solid #999;")
        .arg(currentHighlightColor.name()));

    QPushButton* pickBtn = new QPushButton(QString::fromUtf8("🎨"), annotationPanel);
    pickBtn->setFixedSize(30,30);
    pickBtn->setStyleSheet(
        "QPushButton{background:#333;color:white;font-size:14px;border-radius:6px;}"
        "QPushButton:pressed{background:#555;}");
    connect(pickBtn, &QPushButton::clicked, this, [this](){
        QColor c = QColorDialog::getColor(currentHighlightColor, this, "Cor do Grifador");
        if (c.isValid()) {
            currentHighlightColor = c;
            hlColorPreview->setStyleSheet(
                QString("background:%1;border-radius:4px;border:2px solid #999;").arg(c.name()));
        }
    });
    colorRow->addWidget(colorLbl);
    colorRow->addWidget(hlColorPreview);
    colorRow->addWidget(pickBtn);
    colorRow->addStretch();
    pl->addLayout(colorRow);

    QHBoxLayout* qcRow = new QHBoxLayout();
    qcRow->setSpacing(5);
    for (const QColor& cc : QList<QColor>{
         QColor(255,235,59), QColor(76,175,80),   QColor(33,150,243),
         QColor(244,67,54),  QColor(255,152,0),   QColor(156,39,176)}) {
        QPushButton* cb = new QPushButton(annotationPanel);
        cb->setFixedSize(28,28);
        cb->setStyleSheet(
            QString("QPushButton{background:%1;border-radius:14px;border:2px solid #555;}"
                    "QPushButton:pressed{border-color:white;}").arg(cc.name()));
        connect(cb, &QPushButton::clicked, this, [this,cc](){
            currentHighlightColor = cc;
            hlColorPreview->setStyleSheet(
                QString("background:%1;border-radius:4px;border:2px solid #999;").arg(cc.name()));
        });
        qcRow->addWidget(cb);
    }
    pl->addLayout(qcRow);

    // Grifo simples
    QPushButton* hlBtn = new QPushButton(QString::fromUtf8("🖍  Grifo Simples"), annotationPanel);
    hlBtn->setFixedHeight(40);
    hlBtn->setStyleSheet(
        "QPushButton{background:#1e5c28;color:white;padding:8px;border-radius:7px;font-size:13px;}"
        "QPushButton:pressed{background:#2a7a38;}");
    connect(hlBtn, &QPushButton::clicked, this, [this](){
        if (pendingSelPageRects.isEmpty()) return;
        annotManager->addHighlight(currentTitle, pendingSelPage,
                                    pendingSelText, currentHighlightColor,
                                    pendingSelPageRects);
        reloadPageHighlights(pendingSelPage);
        closeAnnotationPanel();
    });
    pl->addWidget(hlBtn);

    // Grifo + nota
    QPushButton* noteBtn = new QPushButton(QString::fromUtf8("📝  Grifo + Anotação"), annotationPanel);
    noteBtn->setFixedHeight(40);
    noteBtn->setStyleSheet(
        "QPushButton{background:#1e3a6a;color:white;padding:8px;border-radius:7px;font-size:13px;}"
        "QPushButton:pressed{background:#2e4a8a;}");
    connect(noteBtn, &QPushButton::clicked, this, [this](){
        openNoteDialog(-1, pendingSelText, pendingSelPageRects, pendingSelPage, "");
    });
    pl->addWidget(noteBtn);

    QPushButton* closeBtn = new QPushButton(QString::fromUtf8("✕  Fechar"), annotationPanel);
    closeBtn->setFixedHeight(34);
    closeBtn->setStyleSheet(
        "QPushButton{background:#2a2a2a;color:#aaa;border-radius:6px;font-size:12px;}"
        "QPushButton:pressed{background:#444;}");
    connect(closeBtn, &QPushButton::clicked, this, [this](){ closeAnnotationPanel(); });
    pl->addWidget(closeBtn);
    pl->addStretch();
}

// ── Abertura / Fechamento ─────────────────────────────────────────────────────

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

    topBar->show(); bottomBar->show(); topBarVisible = true;
    windowStart = qMax(0, currentPage - 1);

    QTimer::singleShot(0, this, [this](){
        renderVisiblePages();
        QTimer::singleShot(80, this, [this](){
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
    for (auto* w : pageWidgets){ pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();
    bookOpen = false; currentPage = 0; totalPages = 0;
    if (pageInfoLabel) pageInfoLabel->clear();
    if (progressLabel) progressLabel->setText("0%");
}

// ── Renderização ──────────────────────────────────────────────────────────────

void ReaderScreen::renderVisiblePages()
{
    if (!bookOpen) return;
    for (auto* w : pageWidgets){ pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();

    int availW = scrollArea->viewport()->width();
    if (availW < 50){ QTimer::singleShot(20,this,&ReaderScreen::renderVisiblePages); return; }

    const int end = qMin(windowStart + MAX_LOADED, totalPages);

    auto textLoader = std::make_unique<PDFLoader>();
    const bool textOk = textLoader->openDocument(currentFilePath);

    for (int p = windowStart; p < end; p++) {
        PageWidget* pw = new PageWidget(pagesContainer);
        pw->setAmberIntensity(amberIntensity);
        pw->setSepiaEnabled(sepiaEnabled);

        const int rW = qRound(availW * zoomFactor);
        const int rH = qRound(rW * 1.414f);
        QPixmap pix = renderer->renderPage(p, rW, rH);

        if (!pix.isNull()) {
            const QPixmap scaled = pix.scaled(rW, rH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            pw->setPagePixmap(scaled);
            pw->setFixedSize(scaled.size());

            if (textOk) {
                const QSizeF ps = TextExtractor::pageSize(
                    textLoader->getContext(), textLoader->getDocument(), p);
                if (ps.width() > 0 && ps.height() > 0) {
                    pw->setRenderScale(float(scaled.width())  / float(ps.width()),
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

        const int pageIndex = p;

        // FIX: tap no PageWidget → toggle topbar (não depende mais de eventFilter no viewport)
        connect(pw, &PageWidget::pageTapped, this, &ReaderScreen::onPageTapped);

        connect(pw, &PageWidget::selectionFinished, this,
            [this, pageIndex](const QString& text, const QList<QRectF>& rects){
                onSelectionFinished(text, rects, pageIndex);
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
    for (const Annotation& a : anns) {
        if ((a.type == Annotation::Highlight || a.type == Annotation::Note)
            && !a.rects.isEmpty()) {
            PageHighlight hl;
            hl.pageRects    = a.rects;
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

// ── Toggle topbar via sinal pageTapped ────────────────────────────────────────

void ReaderScreen::onPageTapped()
{
    topBarVisible = !topBarVisible;
    topBar->setVisible(topBarVisible);
    bottomBar->setVisible(topBarVisible);
    if (annotPanelVisible && !topBarVisible) closeAnnotationPanel();
}

// ── Painel de anotação ────────────────────────────────────────────────────────

void ReaderScreen::onSelectionFinished(const QString& text,
                                        const QList<QRectF>& pageRects,
                                        int pageIndex)
{
    if (pageRects.isEmpty()) return;
    pendingSelText      = text;
    pendingSelPageRects = pageRects;
    pendingSelPage      = pageIndex;

    const QString preview = text.isEmpty()
        ? "(área selecionada na página)"
        : text.left(110) + (text.length()>110?"…":"");
    selTextPreview->setText(preview);
    showAnnotationPanel();
}

void ReaderScreen::showAnnotationPanel()
{
    annotPanelVisible = true;
    const int topH = topBar->isVisible()    ? topBar->height()    : 0;
    const int botH = bottomBar->isVisible() ? bottomBar->height() : 0;
    annotationPanel->setGeometry(width()-275, topH, 275, height()-topH-botH);
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
            "Arraste sobre o texto para selecionar,\n"
            "depois toque em Grifo Simples ou Grifo + Anotação.");
        pendingSelText.clear(); pendingSelPageRects.clear();
        showAnnotationPanel();
    }
}

// ── Dialog de nota ────────────────────────────────────────────────────────────

void ReaderScreen::openNoteDialog(int annotId,
                                   const QString&       selText,
                                   const QList<QRectF>& pageRects,
                                   int                  pageIndex,
                                   const QString&       existingNote)
{
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle(annotId<0 ? "Nova Anotação" : "Editar Anotação");
    dlg->setStyleSheet("background:#1a1a1a;color:white;");
    dlg->resize(500, 340);

    QVBoxLayout* dl = new QVBoxLayout(dlg);
    dl->setSpacing(10);

    if (!selText.isEmpty()) {
        QLabel* cit = new QLabel(selText.left(160)+(selText.length()>160?"…":""), dlg);
        cit->setStyleSheet(
            QString("color:#eee;background:%1;padding:8px;border-radius:5px;"
                    "font-size:12px;border-left:4px solid white;")
            .arg(currentHighlightColor.darker(120).name()));
        cit->setWordWrap(true);
        dl->addWidget(cit);
    }

    QLabel* noteLbl = new QLabel("Anotação:", dlg);
    noteLbl->setStyleSheet("color:#aaa;font-size:12px;");
    dl->addWidget(noteLbl);

    QTextEdit* te = new QTextEdit(dlg);
    te->setStyleSheet(
        "QTextEdit{background:#252525;color:white;border:1px solid #3a3a3a;"
        "border-radius:5px;padding:6px;font-size:13px;}");
    te->setPlaceholderText("Escreva sua anotação aqui...");
    te->setText(existingNote);
    dl->addWidget(te, 1);

    QHBoxLayout* bRow = new QHBoxLayout();
    QPushButton* ok  = new QPushButton(annotId<0?"✓ Salvar Grifo":"✓ Salvar", dlg);
    ok->setStyleSheet("QPushButton{background:#1e6432;color:white;padding:10px;border-radius:6px;}");
    QPushButton* cn  = new QPushButton("Cancelar", dlg);
    cn->setStyleSheet("QPushButton{background:#333;color:white;padding:10px;border-radius:6px;}");
    connect(ok, &QPushButton::clicked, dlg, &QDialog::accept);
    connect(cn, &QPushButton::clicked, dlg, &QDialog::reject);
    bRow->addWidget(cn); bRow->addWidget(ok);
    dl->addLayout(bRow);

    if (dlg->exec() == QDialog::Accepted) {
        const QString note = te->toPlainText().trimmed();
        if (annotId < 0)
            annotManager->addNote(currentTitle, pageIndex,
                                   note, selText, currentHighlightColor, pageRects);
        else
            annotManager->updateNote(annotId, note);
        reloadPageHighlights(pageIndex);
    }
    dlg->deleteLater();
    closeAnnotationPanel();
}

// ── Lista de anotações ────────────────────────────────────────────────────────

void ReaderScreen::onShowAnnotationsList()
{
    if (!bookOpen) return;
    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Anotações — " + currentTitle);
    dlg->setStyleSheet("background:#1a1a1a;color:white;");
    dlg->resize(660, 540);

    QVBoxLayout* lay = new QVBoxLayout(dlg);
    lay->setSpacing(10);
    QLabel* hdr = new QLabel(QString::fromUtf8("📋  Todas as Anotações"), dlg);
    hdr->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
    lay->addWidget(hdr);

    QListWidget* list = new QListWidget(dlg);
    list->setStyleSheet(
        "QListWidget{background:#111;color:white;border:1px solid #2a2a2a;border-radius:4px;}"
        "QListWidget::item{padding:10px;border-bottom:1px solid #1e1e1e;}"
        "QListWidget::item:selected{background:#1e5c28;}");
    lay->addWidget(list, 1);

    auto reloadList = [&, list](){
        list->clear();
        const auto anns = annotManager->getAllAnnotations(currentTitle);
        if (anns.isEmpty()){
            QListWidgetItem* e = new QListWidgetItem("   Nenhuma anotação ainda.");
            e->setForeground(QColor("#666"));
            list->addItem(e); return;
        }
        for (const Annotation& a : anns) {
            const QString typeStr =
                a.type==Annotation::Highlight ? QString::fromUtf8("🖍  Grifo")
              : a.type==Annotation::Note      ? QString::fromUtf8("📝  Nota")
              : QString::fromUtf8("🔖  Marcador");
            QString body = a.selectedText.left(80);
            if (!a.noteText.isEmpty()) body += "\n     ↳ " + a.noteText.left(60);
            auto* item = new QListWidgetItem(
                QString("[Pág. %1]  %2\n%3").arg(a.pageNumber+1).arg(typeStr).arg(body));
            item->setData(Qt::UserRole,   a.id);
            item->setData(Qt::UserRole+1, a.pageNumber);
            item->setData(Qt::UserRole+2, (int)a.type);
            item->setData(Qt::UserRole+3, a.noteText);
            item->setData(Qt::UserRole+4, a.selectedText);
            if (a.type!=Annotation::Bookmark)
                item->setForeground(a.highlightColor.lighter(160));
            list->addItem(item);
        }
    };
    reloadList();

    QHBoxLayout* btnRow = new QHBoxLayout();

    QPushButton* goBtn = new QPushButton(QString::fromUtf8("↗ Ir para Página"), dlg);
    goBtn->setStyleSheet("QPushButton{background:#1e6432;color:white;padding:8px;border-radius:6px;}");
    connect(goBtn, &QPushButton::clicked, dlg, [this,list,dlg](){
        if (!list->currentItem()||list->count()==0) return;
        const int page = list->currentItem()->data(Qt::UserRole+1).toInt();
        dlg->accept();
        currentPage = page; windowStart = qMax(0,page-1);
        renderVisiblePages();
        QTimer::singleShot(80, this, [this](){
            scrollArea->verticalScrollBar()->setValue(
                scrollPositionForPage(currentPage-windowStart));
        });
    });

    QPushButton* editBtn = new QPushButton(QString::fromUtf8("✎ Editar"), dlg);
    editBtn->setStyleSheet("QPushButton{background:#1e3a6a;color:white;padding:8px;border-radius:6px;}");
    connect(editBtn, &QPushButton::clicked, dlg, [this,list,&reloadList](){
        if (!list->currentItem()||list->count()==0) return;
        const int     id    = list->currentItem()->data(Qt::UserRole).toInt();
        const int     page  = list->currentItem()->data(Qt::UserRole+1).toInt();
        const QString exist = list->currentItem()->data(Qt::UserRole+3).toString();
        const QString sel   = list->currentItem()->data(Qt::UserRole+4).toString();
        QDialog* ed = new QDialog(this);
        ed->setStyleSheet("background:#1a1a1a;color:white;"); ed->resize(480,260);
        QVBoxLayout* el = new QVBoxLayout(ed);
        if (!sel.isEmpty()){ QLabel* c=new QLabel(sel.left(120),ed); c->setWordWrap(true);
            c->setStyleSheet("color:#ccc;background:#252525;padding:6px;border-radius:4px;font-size:11px;");
            el->addWidget(c); }
        QTextEdit* te = new QTextEdit(ed); te->setText(exist);
        te->setStyleSheet("QTextEdit{background:#252525;color:white;border:1px solid #3a3a3a;border-radius:4px;padding:6px;}");
        el->addWidget(te,1);
        QHBoxLayout* eb = new QHBoxLayout();
        QPushButton* sv=new QPushButton("✓ Salvar",ed); sv->setStyleSheet("QPushButton{background:#1e6432;color:white;padding:9px;border-radius:6px;}");
        QPushButton* cn=new QPushButton("Cancelar",ed); cn->setStyleSheet("QPushButton{background:#333;color:white;padding:9px;border-radius:6px;}");
        connect(sv,&QPushButton::clicked,ed,&QDialog::accept);
        connect(cn,&QPushButton::clicked,ed,&QDialog::reject);
        eb->addWidget(cn); eb->addWidget(sv); el->addLayout(eb);
        if (ed->exec()==QDialog::Accepted){ annotManager->updateNote(id,te->toPlainText().trimmed()); reloadPageHighlights(page); reloadList(); }
        ed->deleteLater();
    });

    QPushButton* delBtn = new QPushButton(QString::fromUtf8("🗑 Excluir"), dlg);
    delBtn->setStyleSheet("QPushButton{background:#7a1010;color:white;padding:8px;border-radius:6px;}");
    connect(delBtn, &QPushButton::clicked, dlg, [this,list,&reloadList](){
        if (!list->currentItem()||list->count()==0) return;
        const int id   = list->currentItem()->data(Qt::UserRole).toInt();
        const int page = list->currentItem()->data(Qt::UserRole+1).toInt();
        annotManager->removeAnnotation(id); reloadPageHighlights(page); reloadList();
    });

    QPushButton* closeBtn = new QPushButton("Fechar", dlg);
    closeBtn->setStyleSheet("QPushButton{background:#333;color:white;padding:8px;border-radius:6px;}");
    connect(closeBtn,&QPushButton::clicked,dlg,&QDialog::accept);

    btnRow->addWidget(goBtn); btnRow->addWidget(editBtn); btnRow->addWidget(delBtn);
    btnRow->addStretch(); btnRow->addWidget(closeBtn);
    lay->addLayout(btnRow);
    dlg->exec(); dlg->deleteLater();
}

// ── Expansão de janela deslizante ─────────────────────────────────────────────

void ReaderScreen::expandWindowForward()
{
    if (windowStart + MAX_LOADED >= totalPages) return;
    const int saved = scrollArea->verticalScrollBar()->value();
    const int remH  = pageWidgets.isEmpty() ? 0 : pageWidgets.first()->height()+10;
    windowStart++;
    renderVisiblePages();
    QTimer::singleShot(0,this,[this,saved,remH](){
        scrollArea->verticalScrollBar()->setValue(qMax(0,saved-remH)); });
}

void ReaderScreen::expandWindowBackward()
{
    if (windowStart <= 0) return;
    const int saved = scrollArea->verticalScrollBar()->value();
    windowStart--;
    renderVisiblePages();
    QTimer::singleShot(0,this,[this,saved](){
        const int addH = pageWidgets.isEmpty() ? 0 : pageWidgets.first()->height()+10;
        scrollArea->verticalScrollBar()->setValue(saved+addH); });
}

// ── Scroll debounced ──────────────────────────────────────────────────────────

void ReaderScreen::onScrollDebounced()
{
    if (!bookOpen||pageWidgets.isEmpty()) return;
    const int scrollY = scrollArea->verticalScrollBar()->value();
    int cumY=0, newPage=windowStart;
    for (int i=0; i<pageWidgets.size(); i++) {
        const int h = pageWidgets[i]->height()+10;
        if (scrollY < cumY+h){ newPage=windowStart+i; break; }
        cumY+=h; newPage=windowStart+i;
    }
    if (newPage!=currentPage) {
        currentPage=newPage;
        updatePageInfo(); updateProgressBar();
        progressManager->saveProgress(currentTitle,currentPage);
        bookmarkBtn->setStyleSheet(
            annotManager->hasBookmark(currentTitle,currentPage)
            ? "QPushButton{background:rgba(255,200,0,0.25);color:#FFD700;font-size:18px;border:none;border-radius:18px;}"
            : "QPushButton{background:transparent;color:white;font-size:18px;border:none;}");
    }
    const int distEnd   = (windowStart+MAX_LOADED-1)-currentPage;
    const int distStart = currentPage-windowStart;
    if (distEnd<=0 && windowStart+MAX_LOADED<totalPages) expandWindowForward();
    else if (distStart<=0 && windowStart>0)               expandWindowBackward();
}

int ReaderScreen::scrollPositionForPage(int localIndex) const
{
    int y=0;
    for (int i=0; i<localIndex && i<pageWidgets.size(); i++)
        y += pageWidgets[i]->height()+10;
    return y;
}

// ── Info ──────────────────────────────────────────────────────────────────────

void ReaderScreen::updatePageInfo()
{
    if (!bookOpen||!pageInfoLabel) return;
    const float pct = totalPages>1 ? float(currentPage)/float(totalPages-1)*100.f : 100.f;
    pageInfoLabel->setText(QString("%1/%2 (%3%)").arg(currentPage+1).arg(totalPages).arg(int(pct)));
}

void ReaderScreen::updateProgressBar()
{
    if (!bookOpen||!progressLabel) return;
    const float pct = totalPages>1 ? float(currentPage)/float(totalPages-1)*100.f : 100.f;
    progressLabel->setText(QString("%1%").arg(int(pct)));
}

// ── Tema ──────────────────────────────────────────────────────────────────────

void ReaderScreen::setMenuColor(const QColor& c){ TopBarHelper::setColor(topBar,c); }
void ReaderScreen::applyNightMode(bool e){
    const QString bg = e?"#080808":"#1a1a1a";
    pagesContainer->setStyleSheet(QString("background:%1;").arg(bg));
    scrollArea->setStyleSheet(QString("QScrollArea{border:none;background:%1;}").arg(bg));
}
void ReaderScreen::setAmberIntensity(int v){
    amberIntensity=v; for(auto*pw:pageWidgets) pw->setAmberIntensity(v); }
void ReaderScreen::setSepiaEnabled(bool e){
    sepiaEnabled=e; for(auto*pw:pageWidgets) pw->setSepiaEnabled(e); }

// ── Zoom ──────────────────────────────────────────────────────────────────────

void ReaderScreen::onZoomIn()  { if(zoomFactor<3.0f){ zoomFactor+=0.25f; renderVisiblePages(); } }
void ReaderScreen::onZoomOut() { if(zoomFactor>0.5f){ zoomFactor-=0.25f; renderVisiblePages(); } }

// ── Bookmark ──────────────────────────────────────────────────────────────────

void ReaderScreen::onToggleBookmark()
{
    if (!bookOpen) return;
    if (annotManager->hasBookmark(currentTitle,currentPage)) {
        for (const auto& a : annotManager->getAnnotationsForPage(currentTitle,currentPage))
            if (a.type==Annotation::Bookmark) annotManager->removeAnnotation(a.id);
        bookmarkBtn->setStyleSheet(
            "QPushButton{background:transparent;color:white;font-size:18px;border:none;}");
    } else {
        annotManager->addBookmark(currentTitle,currentPage);
        bookmarkBtn->setStyleSheet(
            "QPushButton{background:rgba(255,200,0,0.25);color:#FFD700;"
            "font-size:18px;border:none;border-radius:18px;}");
    }
}

// ── Teclado ───────────────────────────────────────────────────────────────────

void ReaderScreen::onPreviousPage(){
    if(!bookOpen||currentPage<=0) return;
    currentPage--;
    if(currentPage<windowStart) expandWindowBackward();
    scrollArea->verticalScrollBar()->setValue(scrollPositionForPage(currentPage-windowStart));
}

void ReaderScreen::onNextPage(){
    if(!bookOpen||currentPage>=totalPages-1) return;
    currentPage++;
    if(currentPage>=windowStart+MAX_LOADED) expandWindowForward();
    scrollArea->verticalScrollBar()->setValue(scrollPositionForPage(currentPage-windowStart));
}

void ReaderScreen::keyPressEvent(QKeyEvent* e)
{
    if(!bookOpen){ QWidget::keyPressEvent(e); return; }
    switch(e->key()){
        case Qt::Key_Down: case Qt::Key_PageDown: case Qt::Key_Space: onNextPage();     break;
        case Qt::Key_Up:   case Qt::Key_PageUp:                       onPreviousPage(); break;
        case Qt::Key_Plus: case Qt::Key_Equal: onZoomIn();  break;
        case Qt::Key_Minus:                    onZoomOut(); break;
        case Qt::Key_Escape: emit backClicked(); break;
        default: QWidget::keyPressEvent(e);
    }
}

void ReaderScreen::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
    if(bookOpen) renderVisiblePages();
    if(annotPanelVisible) showAnnotationPanel();
}
