#include "readerscreen.h"
#include "../engine/pdfrenderer.h"
#include "../engine/pdfcache.h"
#include "../storage/progressmanager.h"

#include <QHBoxLayout>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QFileInfo>
#include <QPixmap>
#include <QDebug>

ReaderScreen::ReaderScreen(QWidget* parent)
    : QWidget(parent)
    , renderer(std::make_unique<PDFRenderer>())
    , cache(std::make_unique<PDFCache>(5))
    , progressManager(std::make_unique<ProgressManager>())
    , currentPage(0)
    , totalPages(0)
    , currentFontSize(12)
    , bookOpen(false)
{
    setupUI();
    setFocusPolicy(Qt::StrongFocus);
}

ReaderScreen::~ReaderScreen()
{
    closeBook();
}

void ReaderScreen::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setStyleSheet("background-color: #1e6432;");

    QHBoxLayout* topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    backButton = new QPushButton(QString::fromUtf8("\xe2\x86\x90"), topBar);
    backButton->setFixedSize(40, 40);
    backButton->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 24px; border: none; }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.2); border-radius: 20px; }"
    );
    connect(backButton, &QPushButton::clicked, this, &ReaderScreen::backClicked);

    titleLabel = new QLabel("", topBar);
    titleLabel->setStyleSheet("color: white; font-size: 14px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    pageInfoLabel = new QLabel("", topBar);
    pageInfoLabel->setStyleSheet("color: rgba(255,255,255,0.8); font-size: 12px;");
    pageInfoLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    pageInfoLabel->setFixedWidth(80);

    fontSizeButton = new QPushButton("Aa", topBar);
    fontSizeButton->setFixedSize(40, 40);
    fontSizeButton->setStyleSheet(
        "QPushButton { background: transparent; color: white; font-size: 16px; border: none; }"
        "QPushButton:pressed { background-color: rgba(255,255,255,0.2); border-radius: 20px; }"
    );
    connect(fontSizeButton, &QPushButton::clicked, this, &ReaderScreen::onToggleFontSize);

    topLayout->addWidget(backButton);
    topLayout->addWidget(titleLabel, 1);
    topLayout->addWidget(pageInfoLabel);
    topLayout->addWidget(fontSizeButton);

    mainLayout->addWidget(topBar);

    pageDisplay = new QLabel(this);
    pageDisplay->setAlignment(Qt::AlignCenter);
    pageDisplay->setStyleSheet("background-color: #1a1a1a;");
    pageDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainLayout->addWidget(pageDisplay, 1);
}

void ReaderScreen::openBook(const QString& filePath)
{
    closeBook();

    currentFilePath = filePath;
    QFileInfo info(filePath);
    currentTitle = info.completeBaseName();

    if (renderer->openPDF(filePath)) {
        bookOpen = true;
        totalPages = renderer->getPageCount();
        currentPage = progressManager->getLastPage(currentTitle);

        if (currentPage < 0 || currentPage >= totalPages) {
            currentPage = 0;
        }

        titleLabel->setText(currentTitle);
        renderCurrentPage();
        setFocus();
    } else {
        qDebug() << "Falha ao abrir PDF:" << filePath;
        pageDisplay->setText("Erro ao abrir PDF");
        pageDisplay->setStyleSheet("background-color: #1a1a1a; color: red; font-size: 18px;");
    }
}

void ReaderScreen::closeBook()
{
    if (bookOpen) {
        progressManager->saveProgress(currentTitle, currentPage);
        renderer->closePDF();
        cache->clearCache();
        bookOpen = false;
        currentPage = 0;
        totalPages = 0;
        pageDisplay->clear();
        titleLabel->clear();
        pageInfoLabel->clear();
    }
}

void ReaderScreen::renderCurrentPage()
{
    if (!bookOpen || currentPage < 0 || currentPage >= totalPages) return;

    QPixmap* cached = cache->getCachedPage(currentPage);
    if (cached) {
        pageDisplay->setPixmap(cached->scaled(
            pageDisplay->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
        ));
    } else {
        int w = pageDisplay->width() > 0 ? pageDisplay->width() : 800;
        int h = pageDisplay->height() > 0 ? pageDisplay->height() : 600;
        QPixmap pageImage = renderer->renderPage(currentPage, w, h);

        if (!pageImage.isNull()) {
            cache->cachePageImage(currentPage, pageImage);
            pageDisplay->setPixmap(pageImage.scaled(
                pageDisplay->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation
            ));
        } else {
            pageDisplay->setText("Erro ao renderizar pagina");
        }
    }

    updatePageInfo();
    progressManager->saveProgress(currentTitle, currentPage);
}

void ReaderScreen::updatePageInfo()
{
    pageInfoLabel->setText(QString("%1/%2").arg(currentPage + 1).arg(totalPages));
}

void ReaderScreen::onPreviousPage()
{
    if (bookOpen && currentPage > 0) {
        currentPage--;
        renderCurrentPage();
    }
}

void ReaderScreen::onNextPage()
{
    if (bookOpen && currentPage < totalPages - 1) {
        currentPage++;
        renderCurrentPage();
    }
}

void ReaderScreen::onToggleFontSize()
{
    currentFontSize = (currentFontSize == 12) ? 16 : 12;
    if (bookOpen) {
        cache->clearCache();
        renderCurrentPage();
    }
}

void ReaderScreen::wheelEvent(QWheelEvent* event)
{
    if (!bookOpen) return;

    if (event->angleDelta().y() < 0) {
        onNextPage();
    } else if (event->angleDelta().y() > 0) {
        onPreviousPage();
    }
    event->accept();
}

void ReaderScreen::keyPressEvent(QKeyEvent* event)
{
    if (!bookOpen) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
        case Qt::Key_Right:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
        case Qt::Key_Space:
            onNextPage();
            break;
        case Qt::Key_Left:
        case Qt::Key_Up:
        case Qt::Key_PageUp:
            onPreviousPage();
            break;
        case Qt::Key_Home:
            currentPage = 0;
            renderCurrentPage();
            break;
        case Qt::Key_End:
            currentPage = totalPages - 1;
            renderCurrentPage();
            break;
        case Qt::Key_Escape:
            emit backClicked();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}