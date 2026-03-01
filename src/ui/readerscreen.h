#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <memory>

class PDFRenderer;
class PDFCache;
class ProgressManager;

class ReaderScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ReaderScreen(QWidget* parent = nullptr);
    ~ReaderScreen();

    void openBook(const QString& filePath);
    void closeBook();

signals:
    void backClicked();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onPreviousPage();
    void onNextPage();
    void onToggleFontSize();

private:
    void setupUI();
    void renderCurrentPage();
    void updatePageInfo();

    QVBoxLayout* mainLayout;
    QWidget* topBar;
    QPushButton* backButton;
    QLabel* titleLabel;
    QLabel* pageInfoLabel;
    QPushButton* fontSizeButton;
    QLabel* pageDisplay;

    std::unique_ptr<PDFRenderer> renderer;
    std::unique_ptr<PDFCache> cache;
    std::unique_ptr<ProgressManager> progressManager;

    QString currentFilePath;
    QString currentTitle;
    int currentPage;
    int totalPages;
    int currentFontSize;
    bool bookOpen;
};