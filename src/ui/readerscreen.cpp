#include "readerscreen.h"
#include "topbar_helper.h"
#include "../engine/pdfrenderer.h"
#include "../engine/pdfcache.h"
#include "../storage/progressmanager.h"
#include "../storage/annotationmanager.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QFileInfo>
#include <QInputDialog>
#include <QColorDialog>
#include <QDialog>
#include <QListWidget>
#include <QScrollBar>
#include <QTimer>
#include <QDebug>

// ═══════════════════════════════════════════════════════════════════════════════
// PageWidget
// ═══════════════════════════════════════════════════════════════════════════════

PageWidget::PageWidget(QWidget* parent) : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    pressTimer.setSingleShot(true);
    pressTimer.setInterval(600);
    connect(&pressTimer, &QTimer::timeout, this, [this]() {
        emit longPressed(pressPos);
    });
}

void PageWidget::setPagePixmap(const QPixmap& px) { setPixmap(px); update(); }
void PageWidget::setAmberIntensity(int i) { amberIntensity = i; update(); }
void PageWidget::setSepiaEnabled(bool e)  { sepiaEnabled = e; update(); }

void PageWidget::paintEvent(QPaintEvent* e)
{
    QLabel::paintEvent(e);
    if (amberIntensity <= 0 && !sepiaEnabled) return;
    QPainter p(this);
    if (sepiaEnabled) {
        p.fillRect(rect(), QColor(120, 80, 20, 55));
    }
    if (amberIntensity > 0) {
        // Âmbar real: sobreposição laranja-quente
        int alpha = qBound(0, qRound(amberIntensity * 1.8), 200);
        p.fillRect(rect(), QColor(255, 140, 0, alpha));
    }
}

void PageWidget::mousePressEvent(QMouseEvent* e)
{
    pressPos = e->pos();
    pressTimer.start();
    QLabel::mousePressEvent(e);
}

void PageWidget::mouseReleaseEvent(QMouseEvent* e)
{
    pressTimer.stop();
    QLabel::mouseReleaseEvent(e);
}

// ═══════════════════════════════════════════════════════════════════════════════
// ReaderScreen
// ═══════════════════════════════════════════════════════════════════════════════

ReaderScreen::ReaderScreen(QWidget* parent)
    : QWidget(parent)
    , renderer(std::make_unique<PDFRenderer>())
    , cache(std::make_unique<PDFCache>(4))
    , progressManager(std::make_unique<ProgressManager>())
    , annotManager(std::make_unique<AnnotationManager>())
{
    setupUI();
    setFocusPolicy(Qt::StrongFocus);
}

ReaderScreen::~ReaderScreen() { closeBook(); }

// ── Setup ────────────────────────────────────────────────────────────────────

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

    // Adiciona label de info de página à topbar existente
    pageInfoLabel = new QLabel("", topBar);
    pageInfoLabel->setStyleSheet("color:rgba(255,255,255,0.85);font-size:11px;min-width:80px;");
    pageInfoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    // Insere antes do annotBtn
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

    // Instala filtro de evento para capturar toque no viewport
    scrollArea->viewport()->installEventFilter(this);

    // Conecta scroll para atualizar página — usa timer para debounce
    scrollDebounce = new QTimer(this);
    scrollDebounce->setSingleShot(true);
    scrollDebounce->setInterval(80); // 80ms debounce
    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [this](int) { scrollDebounce->start(); });
    connect(scrollDebounce, &QTimer::timeout, this, &ReaderScreen::onScrollDebounced);

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

    lay->addWidget(zoomOutBtn);
    lay->addWidget(progressLabel, 1);
    lay->addWidget(zoomInBtn);
    lay->addSpacing(8);
    lay->addWidget(bookmarkBtn);

    mainLayout->addWidget(bottomBar);
}

void ReaderScreen::setupAnnotationPanel()
{
    annotationPanel = new QWidget(this);
    annotationPanel->setFixedWidth(270);
    annotationPanel->setStyleSheet("background:#1e1e1e;border-left:1px solid #444;");
    annotationPanel->hide();

    QVBoxLayout* pl = new QVBoxLayout(annotationPanel);
    pl->setContentsMargins(12, 12, 12, 12);
    pl->setSpacing(10);

    QLabel* title = new QLabel("✏️ Anotações", annotationPanel);
    title->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
    pl->addWidget(title);

    // Separador
    QFrame* sep = new QFrame(annotationPanel);
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color:#444;");
    pl->addWidget(sep);

    // ── Marcador com cor escolhida pelo usuário ───────────────────────────────
    QLabel* hlLabel = new QLabel("Marcador:", annotationPanel);
    hlLabel->setStyleSheet("color:#ccc;font-size:13px;");
    pl->addWidget(hlLabel);

    // Preview da cor atual do marcador
    highlightColorPreview = new QLabel(annotationPanel);
    highlightColorPreview->setFixedSize(32, 32);
    currentHighlightColor = QColor(255, 235, 59); // amarelo padrão
    highlightColorPreview->setStyleSheet(
        QString("background:%1;border-radius:4px;border:2px solid #888;")
        .arg(currentHighlightColor.name()));

    QPushButton* chooseColorBtn = new QPushButton("🎨 Escolher Cor do Marcador", annotationPanel);
    chooseColorBtn->setStyleSheet(
        "QPushButton{background:#333;color:white;padding:8px;border-radius:6px;font-size:12px;}"
        "QPushButton:pressed{background:#555;}");
    connect(chooseColorBtn, &QPushButton::clicked, this, [this]() {
        QColor c = QColorDialog::getColor(currentHighlightColor, this, "Cor do Marcador");
        if (c.isValid()) {
            currentHighlightColor = c;
            highlightColorPreview->setStyleSheet(
                QString("background:%1;border-radius:4px;border:2px solid #888;").arg(c.name()));
        }
    });

    QPushButton* addHighlightBtn = new QPushButton("🖍 Marcar Página Atual", annotationPanel);
    addHighlightBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#2a8040;}");
    connect(addHighlightBtn, &QPushButton::clicked, this, [this]() {
        onAddHighlight(currentHighlightColor);
    });

    QHBoxLayout* clrLay = new QHBoxLayout();
    clrLay->addWidget(highlightColorPreview);
    clrLay->addWidget(chooseColorBtn, 1);
    pl->addLayout(clrLay);
    pl->addWidget(addHighlightBtn);

    // ── Nota de texto ─────────────────────────────────────────────────────────
    QPushButton* noteBtn = new QPushButton("📝 Adicionar Nota de Texto", annotationPanel);
    noteBtn->setStyleSheet(
        "QPushButton{background:#2a5080;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#3a6090;}");
    connect(noteBtn, &QPushButton::clicked, this, &ReaderScreen::onAddNote);
    pl->addWidget(noteBtn);

    // ── Ver todas ──────────────────────────────────────────────────────────────
    QPushButton* listBtn = new QPushButton("📋 Ver Todas as Anotações", annotationPanel);
    listBtn->setStyleSheet(noteBtn->styleSheet());
    connect(listBtn, &QPushButton::clicked, this, &ReaderScreen::onShowAnnotationsList);
    pl->addWidget(listBtn);

    pl->addStretch();

    // Botão fechar painel
    QPushButton* closeBtn = new QPushButton("✕ Fechar", annotationPanel);
    closeBtn->setStyleSheet(
        "QPushButton{background:#333;color:white;padding:6px;border-radius:6px;}"
        "QPushButton:pressed{background:#555;}");
    connect(closeBtn, &QPushButton::clicked, this, [this]() {
        annotationPanel->hide();
        annotPanelVisible = false;
    });
    pl->addWidget(closeBtn);
}

// ── Abertura / Fechamento ─────────────────────────────────────────────────────

void ReaderScreen::openBook(const QString& filePath)
{
    closeBook();
    currentFilePath = filePath;
    currentTitle    = QFileInfo(filePath).completeBaseName();

    if (!renderer->openPDF(filePath)) {
        qDebug() << "Falha ao abrir:" << filePath;
        return;
    }

    bookOpen    = true;
    totalPages  = renderer->getPageCount();
    currentPage = progressManager->getLastPage(currentTitle);
    if (currentPage < 0 || currentPage >= totalPages) currentPage = 0;

    topBar->show();
    bottomBar->show();
    topBarVisible = true;

    windowStart = qMax(0, currentPage - 1);
    renderVisiblePages();

    QTimer::singleShot(150, this, [this]() {
        int localIdx = currentPage - windowStart;
        scrollArea->verticalScrollBar()->setValue(scrollPositionForPage(localIdx));
    });

    setFocus();
}

void ReaderScreen::closeBook()
{
    if (!bookOpen) return;
    progressManager->saveProgress(currentTitle, currentPage);
    renderer->closePDF();
    cache->clearCache();

    for (auto* w : pageWidgets) { pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();

    bookOpen    = false;
    currentPage = 0;
    totalPages  = 0;
    pageInfoLabel->clear();
    progressLabel->setText("0%");
}

// ── Renderização: janela deslizante de MAX_LOADED páginas ────────────────────
// FIX BUG: ao trocar janela, preserva a posição de scroll relativa

void ReaderScreen::renderVisiblePages()
{
    if (!bookOpen) return;

    // Remove widgets antigos SEM deletar o container
    for (auto* w : pageWidgets) {
        pagesLayout->removeWidget(w);
        w->deleteLater();
    }
    pageWidgets.clear();

    int end    = qMin(windowStart + MAX_LOADED, totalPages);
    int availW = scrollArea->viewport()->width();
    if (availW < 10) availW = width() > 0 ? width() : 800;

    for (int p = windowStart; p < end; p++) {
        PageWidget* pw = new PageWidget(pagesContainer);
        pw->setAmberIntensity(amberIntensity);
        pw->setSepiaEnabled(sepiaEnabled);

        int renderW = qRound(availW * zoomFactor);
        int renderH = qRound(renderW * 1.414f);

        QPixmap* cached = cache->getCachedPage(p);
        QPixmap  pix;
        if (cached) {
            pix = *cached;
        } else {
            pix = renderer->renderPage(p, renderW, renderH);
            if (!pix.isNull()) cache->cachePageImage(p, pix);
        }

        if (!pix.isNull()) {
            QPixmap scaled = pix.scaled(renderW, renderH,
                                         Qt::KeepAspectRatio, Qt::SmoothTransformation);
            pw->setPagePixmap(scaled);
            pw->setFixedSize(scaled.size());
        } else {
            pw->setText(QString("Página %1").arg(p + 1));
            pw->setFixedSize(renderW, renderH);
        }

        connect(pw, &PageWidget::longPressed, this, [this, p](const QPoint&) {
            currentPage = p;
            if (!annotPanelVisible) onToggleAnnotationPanel();
        });

        pagesLayout->addWidget(pw, 0, Qt::AlignHCenter);
        pageWidgets.append(pw);
    }

    updatePageInfo();
    updateProgressBar();
}

// FIX BUG PRINCIPAL: ao expandir a janela, não reposiciona o scroll (deixa onde está)
void ReaderScreen::expandWindowForward()
{
    if (windowStart + MAX_LOADED >= totalPages) return;

    // Salva posição atual de scroll
    int savedScroll = scrollArea->verticalScrollBar()->value();

    // Calcula quanto espaço as páginas atuais ocupam (para compensar após remoção)
    int removedHeight = 0;
    if (!pageWidgets.isEmpty())
        removedHeight = pageWidgets.first()->height() + 8;

    // Move janela uma página à frente
    windowStart++;
    renderVisiblePages();

    // Reposiciona sem "pular" — subtrai a altura da página que foi removida do topo
    QTimer::singleShot(0, this, [this, savedScroll, removedHeight]() {
        int newPos = qMax(0, savedScroll - removedHeight);
        scrollArea->verticalScrollBar()->setValue(newPos);
    });
}

void ReaderScreen::expandWindowBackward()
{
    if (windowStart <= 0) return;

    int savedScroll = scrollArea->verticalScrollBar()->value();
    windowStart--;
    renderVisiblePages();

    // Reposiciona adicionando a altura da nova página inserida no topo
    QTimer::singleShot(0, this, [this, savedScroll]() {
        int addedHeight = pageWidgets.isEmpty() ? 0 : pageWidgets.first()->height() + 8;
        scrollArea->verticalScrollBar()->setValue(savedScroll + addedHeight);
    });
}

// ── Scroll debounced ──────────────────────────────────────────────────────────

void ReaderScreen::onScrollDebounced()
{
    if (!bookOpen || pageWidgets.isEmpty()) return;

    int scrollY = scrollArea->verticalScrollBar()->value();
    int cumY    = 0;
    int newPage = windowStart;

    for (int i = 0; i < pageWidgets.size(); i++) {
        int pageH = pageWidgets[i]->height() + 8;
        if (scrollY < cumY + pageH) {
            newPage = windowStart + i;
            break;
        }
        cumY += pageH;
        newPage = windowStart + i; // última página visível
    }

    if (newPage != currentPage) {
        currentPage = newPage;
        updatePageInfo();
        updateProgressBar();
        progressManager->saveProgress(currentTitle, currentPage);
    }

    // FIX: expande janela apenas quando o scroll chegou perto do fim/início da janela
    int distToEnd   = (windowStart + MAX_LOADED - 1) - currentPage;
    int distToStart = currentPage - windowStart;

    if (distToEnd <= 0 && windowStart + MAX_LOADED < totalPages) {
        expandWindowForward();
    } else if (distToStart <= 0 && windowStart > 0) {
        expandWindowBackward();
    }
}

int ReaderScreen::scrollPositionForPage(int localIndex) const
{
    int y = 0;
    for (int i = 0; i < localIndex && i < pageWidgets.size(); i++)
        y += pageWidgets[i]->height() + 8;
    return y;
}

// ── Info / progresso ─────────────────────────────────────────────────────────

void ReaderScreen::updatePageInfo()
{
    if (!bookOpen) return;
    float pct = totalPages > 1 ? float(currentPage) / float(totalPages - 1) * 100.f : 100.f;
    pageInfoLabel->setText(
        QString("%1/%2 (%3%)").arg(currentPage + 1).arg(totalPages).arg(int(pct)));
}

void ReaderScreen::updateProgressBar()
{
    if (!bookOpen) return;
    float pct = totalPages > 1 ? float(currentPage) / float(totalPages - 1) * 100.f : 100.f;
    progressLabel->setText(QString("%1%").arg(int(pct)));
}

// ── Tema ─────────────────────────────────────────────────────────────────────

void ReaderScreen::setMenuColor(const QColor& color)
{
    TopBarHelper::setColor(topBar, color);
}

void ReaderScreen::applyNightMode(bool enabled)
{
    nightMode = enabled;
    QString bg = enabled ? "#080808" : "#1a1a1a";
    pagesContainer->setStyleSheet(QString("background:%1;").arg(bg));
    scrollArea->setStyleSheet(QString("QScrollArea{border:none;background:%1;}").arg(bg));
}

void ReaderScreen::setAmberIntensity(int v)
{
    amberIntensity = v;
    for (auto* pw : pageWidgets) pw->setAmberIntensity(v);
}

void ReaderScreen::setSepiaEnabled(bool e)
{
    sepiaEnabled = e;
    for (auto* pw : pageWidgets) pw->setSepiaEnabled(e);
}

// ── Zoom ──────────────────────────────────────────────────────────────────────

void ReaderScreen::onZoomIn()
{
    if (zoomFactor >= 3.0f) return;
    zoomFactor += 0.25f;
    cache->clearCache();
    renderVisiblePages();
}

void ReaderScreen::onZoomOut()
{
    if (zoomFactor <= 0.5f) return;
    zoomFactor -= 0.25f;
    cache->clearCache();
    renderVisiblePages();
}

// ── Toggle topbar/bottombar por toque ────────────────────────────────────────
// FIX: eventFilter corrigido — toque simples (não arrasto) alterna as barras

bool ReaderScreen::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == scrollArea->viewport()) {
        if (ev->type() == QEvent::MouseButtonPress) {
            tapStartPos = static_cast<QMouseEvent*>(ev)->pos();
            tapMoved    = false;
        } else if (ev->type() == QEvent::MouseMove) {
            auto* me = static_cast<QMouseEvent*>(ev);
            if ((me->pos() - tapStartPos).manhattanLength() > 10)
                tapMoved = true;
        } else if (ev->type() == QEvent::MouseButtonRelease) {
            if (!tapMoved) {
                // Toque simples → toggle barras
                topBarVisible = !topBarVisible;
                topBar->setVisible(topBarVisible);
                bottomBar->setVisible(topBarVisible);
                if (annotPanelVisible && !topBarVisible) {
                    annotationPanel->hide();
                    annotPanelVisible = false;
                }
            }
            tapMoved = false;
        }
    }
    return QWidget::eventFilter(obj, ev);
}

// ── Navegação por teclado ─────────────────────────────────────────────────────

void ReaderScreen::onPreviousPage()
{
    if (!bookOpen || currentPage <= 0) return;
    currentPage--;
    if (currentPage < windowStart) expandWindowBackward();
    scrollArea->verticalScrollBar()->setValue(
        scrollPositionForPage(currentPage - windowStart));
}

void ReaderScreen::onNextPage()
{
    if (!bookOpen || currentPage >= totalPages - 1) return;
    currentPage++;
    if (currentPage >= windowStart + MAX_LOADED) expandWindowForward();
    scrollArea->verticalScrollBar()->setValue(
        scrollPositionForPage(currentPage - windowStart));
}

void ReaderScreen::keyPressEvent(QKeyEvent* event)
{
    if (!bookOpen) { QWidget::keyPressEvent(event); return; }
    switch (event->key()) {
        case Qt::Key_Down: case Qt::Key_PageDown: case Qt::Key_Space: onNextPage(); break;
        case Qt::Key_Up:   case Qt::Key_PageUp:                       onPreviousPage(); break;
        case Qt::Key_Plus: case Qt::Key_Equal: onZoomIn();  break;
        case Qt::Key_Minus:                    onZoomOut(); break;
        case Qt::Key_Escape: emit backClicked(); break;
        default: QWidget::keyPressEvent(event);
    }
}

void ReaderScreen::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (bookOpen) { cache->clearCache(); renderVisiblePages(); }

    // Reposiciona painel de anotações se visível
    if (annotPanelVisible) {
        annotationPanel->setGeometry(
            width() - 270,
            topBar->isVisible() ? topBar->height() : 0,
            270,
            height() - (topBar->isVisible() ? topBar->height() : 0)
                      - (bottomBar->isVisible() ? bottomBar->height() : 0)
        );
    }
}

// ── Anotações ────────────────────────────────────────────────────────────────

void ReaderScreen::onToggleAnnotationPanel()
{
    annotPanelVisible = !annotPanelVisible;
    if (annotPanelVisible) {
        annotationPanel->setParent(this);
        annotationPanel->setGeometry(
            width() - 270,
            topBar->isVisible() ? topBar->height() : 0,
            270,
            height() - (topBar->isVisible() ? topBar->height() : 0)
                      - (bottomBar->isVisible() ? bottomBar->height() : 0)
        );
        annotationPanel->raise();
        annotationPanel->show();
    } else {
        annotationPanel->hide();
    }
}

void ReaderScreen::onAddHighlight(const QColor& color)
{
    if (!bookOpen) return;
    // Registra marcação para a página atual com a cor escolhida
    annotManager->addHighlight(currentTitle, currentPage,
        QString("Página %1").arg(currentPage + 1), color);

    // Feedback visual: pisca a borda da página atual
    if (currentPage - windowStart < pageWidgets.size()) {
        PageWidget* pw = pageWidgets[currentPage - windowStart];
        QString orig = pw->styleSheet();
        pw->setStyleSheet(QString("border: 3px solid %1;").arg(color.name()));
        QTimer::singleShot(600, pw, [pw, orig]() { pw->setStyleSheet(orig); });
    }
}

void ReaderScreen::onAddNote()
{
    if (!bookOpen) return;
    // QInputDialog abre teclado virtual automaticamente em touchscreen
    bool ok;
    QString note = QInputDialog::getMultiLineText(
        this, "Nova Anotação",
        QString("Nota para página %1:").arg(currentPage + 1),
        "", &ok);
    if (ok && !note.trimmed().isEmpty())
        annotManager->addNote(currentTitle, currentPage, note.trimmed());
}

void ReaderScreen::onToggleBookmark()
{
    if (!bookOpen) return;
    if (annotManager->hasBookmark(currentTitle, currentPage)) {
        auto anns = annotManager->getAnnotationsForPage(currentTitle, currentPage);
        for (const auto& a : anns)
            if (a.type == Annotation::Bookmark) annotManager->removeAnnotation(a.id);
        bookmarkBtn->setStyleSheet(
            "QPushButton{background:transparent;color:white;font-size:18px;border:none;}");
    } else {
        annotManager->addBookmark(currentTitle, currentPage);
        bookmarkBtn->setStyleSheet(
            "QPushButton{background:rgba(255,200,0,0.3);color:#FFD700;font-size:18px;"
            "border:none;border-radius:18px;}");
    }
}

void ReaderScreen::onShowAnnotationsList()
{
    if (!bookOpen) return;
    annotationPanel->hide();
    annotPanelVisible = false;

    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Anotações — " + currentTitle);
    dlg->setStyleSheet("background:#1e1e1e;color:white;");
    dlg->resize(580, 420);

    QVBoxLayout* lay = new QVBoxLayout(dlg);
    QListWidget* list = new QListWidget(dlg);
    list->setStyleSheet(
        "QListWidget{background:#151515;color:white;border:none;}"
        "QListWidget::item{padding:10px;border-bottom:1px solid #333;}"
        "QListWidget::item:selected{background:#1e6432;}");

    auto allAnns = annotManager->getAllAnnotations(currentTitle);
    for (const auto& a : allAnns) {
        QString typeStr = (a.type == Annotation::Highlight) ? "🖍 Marcação" :
                          (a.type == Annotation::Note)      ? "📝 Nota" : "🔖 Marcador";
        QString body = a.noteText.isEmpty() ? a.selectedText : a.noteText;
        QListWidgetItem* item = new QListWidgetItem(
            QString("[P.%1] %2  %3").arg(a.pageNumber + 1).arg(typeStr).arg(body), list);
        item->setData(Qt::UserRole, a.id);
        item->setData(Qt::UserRole + 1, a.pageNumber);
        if (a.type == Annotation::Highlight)
            item->setForeground(a.highlightColor);
        list->addItem(item);
    }

    connect(list, &QListWidget::itemDoubleClicked, this, [this, dlg](QListWidgetItem* item) {
        int page = item->data(Qt::UserRole + 1).toInt();
        dlg->accept();
        currentPage = page;
        windowStart = qMax(0, page - 1);
        renderVisiblePages();
        QTimer::singleShot(100, this, [this]() {
            scrollArea->verticalScrollBar()->setValue(
                scrollPositionForPage(currentPage - windowStart));
        });
    });

    QPushButton* closeBtn = new QPushButton("Fechar", dlg);
    closeBtn->setStyleSheet("background:#333;color:white;padding:8px;border-radius:6px;");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    lay->addWidget(list);
    lay->addWidget(closeBtn);
    dlg->exec();
    dlg->deleteLater();
}
