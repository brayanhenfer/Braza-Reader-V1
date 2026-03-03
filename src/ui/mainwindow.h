#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QTouchEvent>
#include <memory>

class SidebarMenu;
class LibraryScreen;
class ReaderScreen;
class SettingsScreen;
class AboutScreen;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    bool event(QEvent* event) override;

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

private:
    void setupUI();
    void connectSignals();
    void applyTheme();
    void toggleSidebar();
    void handleTouchEvent(QTouchEvent* touchEvent);

    QWidget*        centralWidget;
    QStackedWidget* screenStack;
    SidebarMenu*    sidebar;

    std::unique_ptr<LibraryScreen>  libraryScreen;
    std::unique_ptr<ReaderScreen>   readerScreen;
    std::unique_ptr<SettingsScreen> settingsScreen;
    std::unique_ptr<AboutScreen>    aboutScreen;

    bool sidebarOpen;
};
