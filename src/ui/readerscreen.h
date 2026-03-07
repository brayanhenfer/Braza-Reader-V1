#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>
#include <QColor>
#include <QTimer>
#include <QRectF>
#include <memory>
#include "pagewidget.h"

class PDFRenderer; class PDFLoader; class ProgressManager; class AnnotationManager;

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
    void setAppBgColor(const QColor& color);

signals:
    void backClicked();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onPreviousPage(); void onNextPage();
    void onZoomIn();       void onZoomOut();
    void onScrollDebounced();
    void onShowAnnotationsList();
    void onPageTapped();
    void onAnnotationPanelSave();

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
    void openEditNoteDialog(int annotId, const QString& existingNote, int pageIndex);
    void updatePageInfo();
    int  scrollPositionForPage(int localIndex) const;

    QVBoxLayout* mainLayout    = nullptr;
    QWidget*     topBar        = nullptr;
    QPushButton* backButton    = nullptr;

    QScrollArea*       scrollArea      = nullptr;
    QWidget*           pagesContainer  = nullptr;
    QVBoxLayout*       pagesLayout     = nullptr;
    QList<PageWidget*> pageWidgets;

    QWidget*     bottomBar     = nullptr;
    QPushButton* zoomInBtn     = nullptr;
    QPushButton* zoomOutBtn    = nullptr;
    QLabel*      pageInfoLabel = nullptr;

    QWidget*   annotationPanel  = nullptr;
    QLabel*    selTextPreview   = nullptr;
    QTextEdit* noteEdit         = nullptr;
    bool       annotPanelVisible = false;

    int           pendingAnnotId   = -1;   // >= 0 = editando existente
    QString       pendingSelText;
    QList<QRectF> pendingSelPageRects;
    int           pendingSelPage = -1;
    QTimer*       scrollDebounce = nullptr;

    std::unique_ptr<PDFRenderer>       renderer;
    std::unique_ptr<ProgressManager>   progressManager;
    std::unique_ptr<AnnotationManager> annotManager;

    QString currentFilePath, currentTitle;
    int     currentPage  = 0;
    int     totalPages   = 0;
    float   zoomFactor   = 1.0f;
    bool    bookOpen     = false;
    bool    topBarVisible= true;
    int     amberIntensity = 0;
    bool    sepiaEnabled   = false;
    int     windowStart    = 0;
    static constexpr int MAX_LOADED = 4;
};
