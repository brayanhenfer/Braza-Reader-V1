#include "readerscreen.h"
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
#include <QListWidget>
#include <QScrollBar>
#include <QTimer>
#include <QDebug>

// ═══════════════════════════════════════════════════════════════════════════════
// PageWidget
// ═══════════════════════════════════════════════════════════════════════════════

PageWidget::PageWidget(QWidget* parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignCenter);
    pressTimer.setSingleShot(true);
    pressTimer.setInterval(600); // 600ms para long press
    connect(&pressTimer, &QTimer::timeout, this, [this]() {
        emit longPressed(pressPos);
    });
}

void PageWidget::setPagePixmap(const QPixmap& px)
{
    setPixmap(px);
    update();
}

void PageWidget::setAmberIntensity(int i) { amberIntensity = i; update(); }
void PageWidget::setSepiaEnabled(bool e)  { sepiaEnabled   = e; update(); }

void PageWidget::paintEvent(QPaintEvent* e)
{
    QLabel::paintEvent(e);
    if (amberIntensity <= 0 && !sepiaEnabled) return;

    QPainter p(this);
    if (sepiaEnabled) {
        // Sépia: overlay marrom-âmbar com maior opacidade
        p.fillRect(rect(), QColor(120, 80, 20, 60));
    }
    if (amberIntensity > 0) {
        // Âmbar: laranja quente, opacidade proporcional à intensidade
        int alpha = qRound(amberIntensity * 1.8); // 0–100 → 0–180
        p.fillRect(rect(), QColor(255, 147, 41, alpha));
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
    , cache(std::make_unique<PDFCache>(4))          // cache de 4 páginas
    , progressManager(std::make_unique<ProgressManager>())
    , annotManager(std::make_unique<AnnotationManager>())
{
    setupUI();
    setFocusPolicy(Qt::StrongFocus);
}

ReaderScreen::~ReaderScreen() { closeBook(); }

// ── Setup da UI ───────────────────────────────────────────────────────────────

void ReaderScreen::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupTopBar();
    setupScrollArea();
    setupBottomBar();
    setupAnnotationPanel();

    // Overlay âmbar: widget transparente sobre o scrollArea
    amberOverlay = new QWidget(this);
    amberOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    amberOverlay->setAttribute(Qt::WA_NoSystemBackground);
    amberOverlay->hide();
}

void ReaderScreen::setupTopBar()
{
    topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color: #1e6432;");

    QHBoxLayout* lay = new QHBoxLayout(topBar);
    lay->setContentsMargins(8, 4, 8, 4);

    backButton = new QPushButton("←", topBar);
    backButton->setFixedSize(40, 40);
    backButton->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:22px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(backButton, &QPushButton::clicked, this, &ReaderScreen::backClicked);

    // Logo/título centralizado
    titleLabel = new QLabel("", topBar);
    titleLabel->setStyleSheet("color:white;font-size:13px;font-weight:bold;");
    titleLabel->setAlignment(Qt::AlignCenter);

    pageInfoLabel = new QLabel("", topBar);
    pageInfoLabel->setStyleSheet("color:rgba(255,255,255,0.85);font-size:11px;");
    pageInfoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    pageInfoLabel->setFixedWidth(90);

    annotBtn = new QPushButton("🖊", topBar);
    annotBtn->setFixedSize(36, 36);
    annotBtn->setStyleSheet(
        "QPushButton{background:transparent;color:white;font-size:18px;border:none;}"
        "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:18px;}");
    connect(annotBtn, &QPushButton::clicked, this, &ReaderScreen::onToggleAnnotationPanel);

    lay->addWidget(backButton);
    lay->addWidget(titleLabel, 1);
    lay->addWidget(pageInfoLabel);
    lay->addWidget(annotBtn);

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

    // Detectar página atual pelo scroll
    connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &ReaderScreen::onScrollValueChanged);

    // Toque na tela → esconde/mostra topbar e bottombar
    scrollArea->viewport()->installEventFilter(this);

    mainLayout->addWidget(scrollArea, 1);
}

void ReaderScreen::setupBottomBar()
{
    bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(44);
    bottomBar->setStyleSheet("background:#1a1a1a;border-top:1px solid #333;");

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
    // Painel lateral que desliza da direita
    annotationPanel = new QWidget(this);
    annotationPanel->setFixedWidth(280);
    annotationPanel->setStyleSheet("background:#252525;border-left:1px solid #444;");
    annotationPanel->hide();

    QVBoxLayout* panLay = new QVBoxLayout(annotationPanel);
    panLay->setContentsMargins(12, 12, 12, 12);
    panLay->setSpacing(10);

    QLabel* panTitle = new QLabel("Anotações", annotationPanel);
    panTitle->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
    panLay->addWidget(panTitle);

    // Botões de ação
    struct BtnDef { QString label; QString slot; QColor color; };
    QList<BtnDef> btns = {
        {"🟡 Marcador Amarelo", "", QColor(255,235,59)},
        {"🟢 Marcador Verde",   "", QColor(76,175,80)},
        {"🔵 Marcador Azul",    "", QColor(33,150,243)},
        {"🔴 Marcador Vermelho","", QColor(244,67,54)},
    };

    for (const auto& bd : btns) {
        QPushButton* b = new QPushButton(bd.label, annotationPanel);
        b->setStyleSheet(
            "QPushButton{background:#333;color:white;padding:8px;border-radius:6px;text-align:left;}"
            "QPushButton:pressed{background:#555;}");
        QColor c = bd.color;
        connect(b, &QPushButton::clicked, this, [this, c]() { onAddHighlight(c); });
        panLay->addWidget(b);
    }

    QPushButton* noteBtn = new QPushButton("📝 Adicionar Nota", annotationPanel);
    noteBtn->setStyleSheet(
        "QPushButton{background:#1e6432;color:white;padding:8px;border-radius:6px;}"
        "QPushButton:pressed{background:#2a8040;}");
    connect(noteBtn, &QPushButton::clicked, this, &ReaderScreen::onAddNote);
    panLay->addWidget(noteBtn);

    QPushButton* listBtn = new QPushButton("📋 Ver Todas", annotationPanel);
    listBtn->setStyleSheet(noteBtn->styleSheet());
    connect(listBtn, &QPushButton::clicked, this, &ReaderScreen::onShowAnnotationsList);
    panLay->addWidget(listBtn);

    panLay->addStretch();
}

// ── Abertura / fechamento ─────────────────────────────────────────────────────

void ReaderScreen::openBook(const QString& filePath)
{
    closeBook();
    currentFilePath = filePath;
    currentTitle    = QFileInfo(filePath).completeBaseName();

    if (!renderer->openPDF(filePath)) {
        qDebug() << "Falha ao abrir:" << filePath;
        return;
    }

    bookOpen   = true;
    totalPages = renderer->getPageCount();
    currentPage = progressManager->getLastPage(currentTitle);
    if (currentPage < 0 || currentPage >= totalPages) currentPage = 0;

    titleLabel->setText(currentTitle);
    topBar->show();
    bottomBar->show();
    topBarVisible = true;

    // Inicializa janela deslizante
    windowStart = qMax(0, currentPage - 1);
    renderVisiblePages();

    // Scroll para a página salva
    QTimer::singleShot(100, this, [this]() {
        int pos = scrollPositionForPage(currentPage - windowStart);
        scrollArea->verticalScrollBar()->setValue(pos);
    });

    setFocus();
}

void ReaderScreen::closeBook()
{
    if (!bookOpen) return;
    progressManager->saveProgress(currentTitle, currentPage);
    renderer->closePDF();
    cache->clearCache();

    // Limpa widgets de página
    for (auto* w : pageWidgets) { pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();

    bookOpen    = false;
    currentPage = 0;
    totalPages  = 0;
    titleLabel->clear();
    pageInfoLabel->clear();
    progressLabel->setText("0%");
}

// ── Renderização: janela deslizante de MAX_LOADED páginas ────────────────────

void ReaderScreen::renderVisiblePages()
{
    if (!bookOpen) return;

    // Limpa widgets antigos
    for (auto* w : pageWidgets) { pagesLayout->removeWidget(w); w->deleteLater(); }
    pageWidgets.clear();

    int end = qMin(windowStart + MAX_LOADED, totalPages);
    int availW = scrollArea->viewport()->width();
    if (availW < 10) availW = 800;

    for (int p = windowStart; p < end; p++) {
        PageWidget* pw = new PageWidget(pagesContainer);
        pw->setAmberIntensity(amberIntensity);
        pw->setSepiaEnabled(sepiaEnabled);

        // Largura = viewport * zoom, altura proporcional A4
        int renderW = qRound(availW * zoomFactor);
        int renderH = qRound(renderW * 1.414f); // proporção A4

        QPixmap* cached = cache->getCachedPage(p);
        QPixmap  pix;
        if (cached) {
            pix = *cached;
        } else {
            pix = renderer->renderPage(p, renderW, renderH);
            if (!pix.isNull()) cache->cachePageImage(p, pix);
        }

        if (!pix.isNull()) {
            // Escala para caber no viewport mantendo aspecto
            QPixmap scaled = pix.scaled(renderW, renderH,
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
            pw->setPagePixmap(scaled);
            pw->setFixedSize(scaled.size());
        } else {
            pw->setText(QString("Página %1").arg(p + 1));
            pw->setFixedSize(renderW, renderH);
        }

        // Long press → anotação
        connect(pw, &PageWidget::longPressed, this, [this, p](const QPoint&) {
            currentPage = p;
            onToggleAnnotationPanel();
        });

        pagesLayout->addWidget(pw, 0, Qt::AlignHCenter);
        pageWidgets.append(pw);
    }

    updatePageInfo();
    updateProgressBar();
}

void ReaderScreen::loadPageIntoWidget(int pageIndex)
{
    // Chamado quando scroll chega perto do fim/início da janela
    int newStart = qMax(0, pageIndex - 1);
    if (newStart == windowStart) return;
    windowStart = newStart;
    renderVisiblePages();
}

// ── Scroll → atualiza página atual e expande janela se necessário ─────────────

void ReaderScreen::onScrollValueChanged(int value)
{
    if (!bookOpen || pageWidgets.isEmpty()) return;

    // Descobre qual PageWidget está mais visível
    QScrollBar* sb   = scrollArea->verticalScrollBar();
    int         vMax = sb->maximum();
    if (vMax <= 0) return;

    // Proporção do scroll → página aproximada no documento inteiro
    float ratio = float(value) / float(vMax);
    int   approx = qRound(ratio * (totalPages - 1));

    // Página dentro da janela carregada
    int localPage = windowStart;
    int viewport_top = scrollArea->verticalScrollBar()->value();
    int cumY = 0;
    for (int i = 0; i < pageWidgets.size(); i++) {
        cumY += pageWidgets[i]->height() + 8;
        if (cumY > viewport_top) {
            localPage = windowStart + i;
            break;
        }
    }

    if (localPage != currentPage) {
        currentPage = localPage;
        updatePageInfo();
        updateProgressBar();
        progressManager->saveProgress(currentTitle, currentPage);
    }

    // Expande janela se estiver perto do fim
    int distToEnd = (windowStart + MAX_LOADED) - currentPage;
    if (distToEnd <= 1 && windowStart + MAX_LOADED < totalPages) {
        loadPageIntoWidget(currentPage + 1);
    }
    // Expande para trás se perto do início
    if (currentPage <= windowStart && windowStart > 0) {
        loadPageIntoWidget(currentPage);
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
    pageInfoLabel->setText(QString("%1/%2  (%3%)")
                           .arg(currentPage + 1).arg(totalPages).arg(int(pct)));
}

void ReaderScreen::updateProgressBar()
{
    if (!bookOpen) return;
    float pct = totalPages > 1 ? float(currentPage) / float(totalPages - 1) * 100.f : 100.f;
    progressLabel->setText(QString("%1%").arg(int(pct)));
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

// ── Navegação teclado ─────────────────────────────────────────────────────────

void ReaderScreen::onPreviousPage()
{
    if (!bookOpen || currentPage <= 0) return;
    currentPage--;
    if (currentPage < windowStart) loadPageIntoWidget(currentPage);
    int pos = scrollPositionForPage(currentPage - windowStart);
    scrollArea->verticalScrollBar()->setValue(pos);
}

void ReaderScreen::onNextPage()
{
    if (!bookOpen || currentPage >= totalPages - 1) return;
    currentPage++;
    if (currentPage >= windowStart + MAX_LOADED) loadPageIntoWidget(currentPage);
    int pos = scrollPositionForPage(currentPage - windowStart);
    scrollArea->verticalScrollBar()->setValue(pos);
}

void ReaderScreen::keyPressEvent(QKeyEvent* event)
{
    if (!bookOpen) { QWidget::keyPressEvent(event); return; }
    switch (event->key()) {
        case Qt::Key_Down: case Qt::Key_PageDown: case Qt::Key_Space: onNextPage(); break;
        case Qt::Key_Up:   case Qt::Key_PageUp:                       onPreviousPage(); break;
        case Qt::Key_Plus: case Qt::Key_Equal:  onZoomIn();  break;
        case Qt::Key_Minus:                     onZoomOut(); break;
        case Qt::Key_Escape: emit backClicked(); break;
        default: QWidget::keyPressEvent(event);
    }
}

// ── Resize: re-renderiza com novo tamanho ─────────────────────────────────────

void ReaderScreen::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (bookOpen) {
        cache->clearCache();
        renderVisiblePages();
    }
    // Reposiciona overlay âmbar
    if (amberOverlay) {
        amberOverlay->setGeometry(scrollArea->geometry());
    }
}

// ── Cor do menu ───────────────────────────────────────────────────────────────

void ReaderScreen::setMenuColor(const QColor& color)
{
    topBar->setStyleSheet(QString("background-color:%1;").arg(color.name()));
}

// ── Modo noturno ──────────────────────────────────────────────────────────────
// Implementado via overlay escuro semitransparente sobre todo o widget
void ReaderScreen::applyNightMode(bool enabled)
{
    nightMode = enabled;
    QString bgColor = enabled ? "#0a0a0a" : "#1a1a1a";
    pagesContainer->setStyleSheet(QString("background:%1;").arg(bgColor));
    scrollArea->setStyleSheet(QString("QScrollArea{border:none;background:%1;}").arg(bgColor));
    bottomBar->setStyleSheet(QString("background:%1;border-top:1px solid #333;").arg(
        enabled ? "#050505" : "#1a1a1a"));
}

// ── Filtros visuais ───────────────────────────────────────────────────────────

void ReaderScreen::applyVisualFilters()
{
    for (auto* pw : pageWidgets) {
        pw->setAmberIntensity(amberIntensity);
        pw->setSepiaEnabled(sepiaEnabled);
    }
}

// ── Anotações ────────────────────────────────────────────────────────────────

void ReaderScreen::onToggleAnnotationPanel()
{
    annotPanelVisible = !annotPanelVisible;
    if (annotPanelVisible) {
        annotationPanel->setParent(this);
        annotationPanel->setGeometry(
            width() - 280, topBar->height(),
            280, height() - topBar->height() - bottomBar->height()
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
    // Na prática, o texto selecionado viria de um mecanismo de seleção de texto do MuPDF.
    // Por ora, registramos um marcador de posição para a página atual.
    annotManager->addHighlight(currentTitle, currentPage, "(marcação na página)", color);
    annotationPanel->hide();
    annotPanelVisible = false;
}

void ReaderScreen::onAddNote()
{
    if (!bookOpen) return;
    bool ok;
    QString note = QInputDialog::getMultiLineText(
        this, "Nova Anotação",
        QString("Anotação para página %1:").arg(currentPage + 1),
        "", &ok
    );
    if (ok && !note.trimmed().isEmpty()) {
        annotManager->addNote(currentTitle, currentPage, note.trimmed());
    }
    annotationPanel->hide();
    annotPanelVisible = false;
}

void ReaderScreen::onToggleBookmark()
{
    if (!bookOpen) return;
    if (annotManager->hasBookmark(currentTitle, currentPage)) {
        // Remove bookmark existente
        auto anns = annotManager->getAnnotationsForPage(currentTitle, currentPage);
        for (const auto& a : anns)
            if (a.type == Annotation::Bookmark) annotManager->removeAnnotation(a.id);
        bookmarkBtn->setText("🔖");
    } else {
        annotManager->addBookmark(currentTitle, currentPage);
        bookmarkBtn->setText("🔖✓");
    }
}

void ReaderScreen::onShowAnnotationsList()
{
    if (!bookOpen) return;
    annotationPanel->hide();
    annotPanelVisible = false;

    QDialog* dlg = new QDialog(this);
    dlg->setWindowTitle("Anotações — " + currentTitle);
    dlg->setStyleSheet("background:#252525;color:white;");
    dlg->resize(600, 400);

    QVBoxLayout* lay = new QVBoxLayout(dlg);
    QListWidget* list = new QListWidget(dlg);
    list->setStyleSheet(
        "QListWidget{background:#1e1e1e;color:white;border:none;}"
        "QListWidget::item{padding:8px;border-bottom:1px solid #333;}"
        "QListWidget::item:selected{background:#1e6432;}");

    auto allAnns = annotManager->getAllAnnotations(currentTitle);
    for (const auto& a : allAnns) {
        QString typeStr = (a.type == Annotation::Highlight) ? "🟡 Marcação" :
                          (a.type == Annotation::Note)      ? "📝 Nota" : "🔖 Marcador";
        QString text = QString("[P.%1] %2\n%3")
                       .arg(a.pageNumber + 1)
                       .arg(typeStr)
                       .arg(a.noteText.isEmpty() ? a.selectedText : a.noteText);
        QListWidgetItem* item = new QListWidgetItem(text, list);
        item->setData(Qt::UserRole, a.id);
    }

    // Ir para página ao clicar
    connect(list, &QListWidget::itemDoubleClicked, this, [this, dlg, allAnns](QListWidgetItem* item) {
        int id = item->data(Qt::UserRole).toInt();
        for (const auto& a : allAnns) {
            if (a.id == id) {
                dlg->accept();
                currentPage = a.pageNumber;
                if (currentPage < windowStart || currentPage >= windowStart + MAX_LOADED)
                    loadPageIntoWidget(currentPage);
                int pos = scrollPositionForPage(currentPage - windowStart);
                scrollArea->verticalScrollBar()->setValue(pos);
                break;
            }
        }
    });

    QPushButton* closeBtn = new QPushButton("Fechar", dlg);
    closeBtn->setStyleSheet("background:#333;color:white;padding:8px;border-radius:6px;");
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);

    lay->addWidget(list);
    lay->addWidget(closeBtn);
    dlg->exec();
    dlg->deleteLater();
}

// ── eventFilter para toque na área de scroll → toggle topbar/bottombar ────────
bool ReaderScreen::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == scrollArea->viewport() && ev->type() == QEvent::MouseButtonPress) {
        auto* me = static_cast<QMouseEvent*>(ev);
        // Toque na margem superior da viewport → toggle bars
        if (me->pos().y() < 60) {
            topBarVisible = !topBarVisible;
            topBar->setVisible(topBarVisible);
            bottomBar->setVisible(topBarVisible);
        }
    }
    return QWidget::eventFilter(obj, ev);
}
