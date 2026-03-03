#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QColor>
#include <QTimer>
#include <QPoint>
#include <QRectF>
#include <memory>

#include "pagewidget.h"   // define PageWidget + TextWord + PageHighlight

class PDFRenderer;
class PDFLoader;          // para TextExtractor via getContext()/getDocument()
class ProgressManager;
class AnnotationManager;

class ReaderScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ReaderScreen(QWidget* parent = nullptr);
    ~ReaderScreen();

    void openBook(const QString& filePath);
    void closeBook();
    void setMenuColor(const QColor& color);
    void applyNightMode(bool enabled);
    void setAmberIntensity(int v);
    void setSepiaEnabled(bool e);

signals:
    void backClicked();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
    void onPreviousPage();
    void onNextPage();
    void onZoomIn();
    void onZoomOut();
    void onToggleAnnotationPanel();
    void onScrollDebounced();
    void onToggleBookmark();
    void onShowAnnotationsList();
    void onSelectionFinished(const QString& text,
                              const QList<QRectF>& pageRects,
                              int pageIndex);

private:
    void setupUI();
    void setupTopBar();
    void setupScrollArea();
    void setupBottomBar();
    void setupAnnotationPanel();

    void renderVisiblePages();
    void expandWindowForward();
    void expandWindowBackward();
    void loadHighlightsForPage(PageWidget* pw, int pageIndex);
    void reloadPageHighlights(int pageIndex);
    void showAnnotationPanel();
    void closeAnnotationPanel();
    void openNoteDialog(int annotId,
                        const QString&       selText,
                        const QList<QRectF>& pageRects,
                        int                  pageIndex,
                        const QString&       existingNote);
    void updatePageInfo();
    void updateProgressBar();
    int  scrollPositionForPage(int localIndex) const;

    // ── Layout ──────────────────────────────────────────────────────────────
    QVBoxLayout*       mainLayout   = nullptr;
    QWidget*           topBar       = nullptr;
    QPushButton*       backButton   = nullptr;
    QLabel*            pageInfoLabel = nullptr;
    QPushButton*       annotBtn     = nullptr;

    QScrollArea*       scrollArea      = nullptr;
    QWidget*           pagesContainer  = nullptr;
    QVBoxLayout*       pagesLayout     = nullptr;
    QList<PageWidget*> pageWidgets;

    QWidget*           bottomBar    = nullptr;
    QPushButton*       zoomInBtn    = nullptr;
    QPushButton*       zoomOutBtn   = nullptr;
    QPushButton*       bookmarkBtn  = nullptr;
    QLabel*            progressLabel = nullptr;

    // ── Painel de anotação flutuante ────────────────────────────────────────
    QWidget*           annotationPanel  = nullptr;
    QLabel*            selTextPreview   = nullptr;
    QLabel*            hlColorPreview   = nullptr;
    QColor             currentHighlightColor{255, 235, 59};
    bool               annotPanelVisible = false;

    // Seleção pendente em coords de PÁGINA (PDF pts)
    QString            pendingSelText;
    QList<QRectF>      pendingSelPageRects;
    int                pendingSelPage = -1;

    QTimer*            scrollDebounce = nullptr;
    QPoint             tapStartPos;
    bool               tapMoved = false;

    // ── Engine ──────────────────────────────────────────────────────────────
    std::unique_ptr<PDFRenderer>       renderer;
    // PDFLoader exposto pelo renderer via amigo — acessamos via getter no .cpp
    std::unique_ptr<ProgressManager>   progressManager;
    std::unique_ptr<AnnotationManager> annotManager;

    // ── Estado ──────────────────────────────────────────────────────────────
    QString currentFilePath;
    QString currentTitle;
    int     currentPage    = 0;
    int     totalPages     = 0;
    float   zoomFactor     = 1.0f;
    bool    bookOpen       = false;
    bool    topBarVisible  = true;
    int     amberIntensity = 0;
    bool    sepiaEnabled   = false;

    int  windowStart = 0;
    static constexpr int MAX_LOADED = 4;
};
