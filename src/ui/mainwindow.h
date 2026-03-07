#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QTouchEvent>
#include <QWidget>
#include <memory>

class SidebarMenu; class LibraryScreen; class ReaderScreen;
class SettingsScreen; class AboutScreen; class CollectionScreen; class TermsScreen;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
private slots:
    void onMenuClicked();
    void onNavigateToLibrary(); void onNavigateToFavorites();
    void onNavigateToCollections(); void onNavigateToSettings();
    void onNavigateToTerms(); void onNavigateToAbout();
    void onOpenBook(const QString& filePath); void onCloseReader();
    void onApplySettings();
private:
    void setupUI(); void connectSignals(); void applyTheme(); void toggleSidebar();
    void handleTouchEvent(QTouchEvent* te);
    void applyBrightness(int percent);

    QWidget*        centralWidget;
    QStackedWidget* screenStack;
    SidebarMenu*    sidebar;
    QWidget*        brightnessOverlay = nullptr;   // software brightness dimmer
    std::unique_ptr<LibraryScreen>    libraryScreen;
    std::unique_ptr<ReaderScreen>     readerScreen;
    std::unique_ptr<SettingsScreen>   settingsScreen;
    std::unique_ptr<AboutScreen>      aboutScreen;
    std::unique_ptr<CollectionScreen> collectionScreen;
    std::unique_ptr<TermsScreen>      termsScreen;
    bool sidebarOpen = false;
};
