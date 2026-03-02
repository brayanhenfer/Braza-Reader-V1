#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QTouchEvent>
#include <QColor>
#include <memory>

class SidebarMenu;
class LibraryScreen;
class ReaderScreen;
class SettingsScreen;
class AboutScreen;
class TermsScreen;
class CollectionScreen;
class SettingsManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onMenuClicked();
    void onNavigateToLibrary();
    void onNavigateToFavorites();
    void onNavigateToCollections();
    void onNavigateToSettings();
    void onNavigateToTerms();
    void onNavigateToAbout();
    void onOpenBook(const QString& filePath);
    void onCloseReader();
    void onSettingsChanged();

private:
    void setupUI();
    void connectSignals();
    void applyTheme();
    void toggleSidebar();
    void closeSidebar();
    void handleTouchEvent(QTouchEvent* touchEvent);

    QWidget*         centralWidget;
    QStackedWidget*  screenStack;
    SidebarMenu*     sidebar;

    // Overlay âmbar: cobre TODA a tela (topbars, menus, leitor)
    QWidget*         amberOverlay      = nullptr;
    // Overlay de brilho
    QWidget*         brightnessOverlay = nullptr;

    std::unique_ptr<LibraryScreen>    libraryScreen;
    std::unique_ptr<ReaderScreen>     readerScreen;
    std::unique_ptr<SettingsScreen>   settingsScreen;
    std::unique_ptr<AboutScreen>      aboutScreen;
    std::unique_ptr<TermsScreen>      termsScreen;
    std::unique_ptr<CollectionScreen> collectionScreen;
    std::unique_ptr<SettingsManager>  settingsManager;

    bool sidebarOpen = false;
};
