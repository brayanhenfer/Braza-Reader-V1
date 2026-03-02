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
#include <memory>

class PDFRenderer;
class PDFCache;
class ProgressManager;
class AnnotationManager;

// ─── Widget de página com filtros visuais ────────────────────────────────────
class PageWidget : public QLabel
{
    Q_OBJECT
public:
    explicit PageWidget(QWidget* parent = nullptr);
    void setPagePixmap(const QPixmap& px);
    void setAmberIntensity(int intensity);
    void setSepiaEnabled(bool enabled);

signals:
    void longPressed(const QPoint& pos);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QTimer pressTimer;
    QPoint pressPos;
    int    amberIntensity = 0;
    bool   sepiaEnabled   = false;
};

// ─── Tela de leitura ─────────────────────────────────────────────────────────
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
    void onAddHighlight(const QColor& color);
    void onAddNote();
    void onToggleBookmark();
    void onScrollDebounced();
    void onShowAnnotationsList();

private:
    void setupUI();
    void setupTopBar();
    void setupScrollArea();
    void setupBottomBar();
    void setupAnnotationPanel();

    void renderVisiblePages();
    void expandWindowForward();
    void expandWindowBackward();
    void updatePageInfo();
    void updateProgressBar();
    int  scrollPositionForPage(int localIndex) const;

    // Layout
    QVBoxLayout*       mainLayout;
    QWidget*           topBar;
    QPushButton*       backButton;
    QLabel*            pageInfoLabel;
    QPushButton*       annotBtn;

    QScrollArea*       scrollArea;
    QWidget*           pagesContainer;
    QVBoxLayout*       pagesLayout;
    QList<PageWidget*> pageWidgets;

    QWidget*           bottomBar;
    QPushButton*       zoomInBtn;
    QPushButton*       zoomOutBtn;
    QPushButton*       bookmarkBtn;
    QLabel*            progressLabel;

    QWidget*           annotationPanel;
    QLabel*            highlightColorPreview;
    QColor             currentHighlightColor;
    bool               annotPanelVisible = false;

    QTimer*            scrollDebounce;

    // Estado de toque (para detectar tap vs scroll)
    QPoint             tapStartPos;
    bool               tapMoved = false;

    // Engine
    std::unique_ptr<PDFRenderer>       renderer;
    std::unique_ptr<PDFCache>          cache;
    std::unique_ptr<ProgressManager>   progressManager;
    std::unique_ptr<AnnotationManager> annotManager;

    QString currentFilePath;
    QString currentTitle;
    int     currentPage    = 0;
    int     totalPages     = 0;
    float   zoomFactor     = 1.0f;
    bool    bookOpen       = false;
    bool    topBarVisible  = true;
    bool    nightMode      = false;
    int     amberIntensity = 0;
    bool    sepiaEnabled   = false;

    int windowStart = 0;
    static constexpr int MAX_LOADED = 4;
};
