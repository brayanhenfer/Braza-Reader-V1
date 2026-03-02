#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSlider>
#include <QColor>
#include <QTimer>
#include <memory>

class PDFRenderer;
class PDFCache;
class ProgressManager;
class AnnotationManager;

// ─── Widget interno: exibe uma única página com overlay de anotações ──────────
class PageWidget : public QLabel
{
    Q_OBJECT
public:
    explicit PageWidget(QWidget* parent = nullptr);
    void setPagePixmap(const QPixmap& px);
    void setAmberIntensity(int intensity);  // 0–100
    void setSepiaEnabled(bool enabled);

signals:
    void longPressed(const QPoint& pos);

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    QTimer  pressTimer;
    QPoint  pressPos;
    int     amberIntensity = 0;
    bool    sepiaEnabled   = false;
};

// ─── Tela principal de leitura ────────────────────────────────────────────────
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
    void onAddNote();
    void onAddHighlight(const QColor& color);
    void onToggleBookmark();
    void onScrollValueChanged(int value);
    void onShowAnnotationsList();

private:
    void setupUI();
    void setupTopBar();
    void setupScrollArea();
    void setupBottomBar();
    void setupAnnotationPanel();

    void renderVisiblePages();
    void loadPageIntoWidget(int pageIndex);
    void updatePageInfo();
    void updateProgressBar();
    void applyVisualFilters();
    void showAnnotationDialog(const QString& type);

    int  pageAtScrollPosition() const;
    int  scrollPositionForPage(int page) const;

    // ── Layout ────────────────────────────────────────────────────────────────
    QVBoxLayout*  mainLayout;

    // Topbar (some ao toque)
    QWidget*      topBar;
    QPushButton*  backButton;
    QLabel*       titleLabel;
    QLabel*       pageInfoLabel;    // "12/340  (3.5%)"
    QPushButton*  annotBtn;         // 🖊

    // Área de scroll contínuo
    QScrollArea*  scrollArea;
    QWidget*      pagesContainer;
    QVBoxLayout*  pagesLayout;
    QList<PageWidget*> pageWidgets; // janela deslizante de 4 páginas

    // Barra inferior (zoom + bookmark + filtros)
    QWidget*      bottomBar;
    QPushButton*  zoomInBtn;
    QPushButton*  zoomOutBtn;
    QPushButton*  bookmarkBtn;
    QLabel*       progressLabel;    // "3.5%"

    // Painel lateral de anotações (toggle)
    QWidget*      annotationPanel;
    bool          annotPanelVisible = false;

    // Overlay âmbar (widget semitransparente sobre o scrollArea)
    QWidget*      amberOverlay;

    // ── Estado ────────────────────────────────────────────────────────────────
    std::unique_ptr<PDFRenderer>      renderer;
    std::unique_ptr<PDFCache>         cache;
    std::unique_ptr<ProgressManager>  progressManager;
    std::unique_ptr<AnnotationManager> annotManager;

    QString currentFilePath;
    QString currentTitle;
    int     currentPage      = 0;
    int     totalPages       = 0;
    float   zoomFactor       = 1.0f;
    bool    bookOpen         = false;
    bool    topBarVisible    = true;
    bool    nightMode        = false;
    int     amberIntensity   = 0;    // 0–100
    bool    sepiaEnabled     = false;
    int     brightness       = 100;  // 0–100

    // Janela deslizante: quais páginas estão carregadas no layout
    int     windowStart      = 0;    // primeira página carregada
    static constexpr int MAX_LOADED = 4; // máximo 4 páginas em memória
};
