#include "mainwindow.h"
#include "sidebarmenu.h"
#include "libraryscreen.h"
#include "readerscreen.h"
#include "settingsscreen.h"
#include "aboutscreen.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTouchEvent>
#include <QPalette>
#include <QEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , sidebarOpen(false)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    setWindowState(Qt::WindowFullScreen);

    setupUI();
    connectSignals();
    applyTheme();

    libraryScreen->loadLibrary();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    sidebar = new SidebarMenu(this);
    sidebar->setMaximumWidth(230);
    sidebar->hide();

    screenStack = new QStackedWidget(this);

    libraryScreen  = std::make_unique<LibraryScreen>();
    readerScreen   = std::make_unique<ReaderScreen>();
    settingsScreen = std::make_unique<SettingsScreen>();
    aboutScreen    = std::make_unique<AboutScreen>();

    screenStack->addWidget(libraryScreen.get());
    screenStack->addWidget(readerScreen.get());
    screenStack->addWidget(settingsScreen.get());
    screenStack->addWidget(aboutScreen.get());
    screenStack->setCurrentWidget(libraryScreen.get());

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(screenStack, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::connectSignals()
{
    connect(libraryScreen.get(),  &LibraryScreen::menuClicked,  this, &MainWindow::onMenuClicked);
    connect(libraryScreen.get(),  &LibraryScreen::bookOpened,   this, &MainWindow::onOpenBook);
    connect(readerScreen.get(),   &ReaderScreen::backClicked,   this, &MainWindow::onCloseReader);
    connect(settingsScreen.get(), &SettingsScreen::menuClicked, this, &MainWindow::onMenuClicked);
    connect(aboutScreen.get(),    &AboutScreen::menuClicked,    this, &MainWindow::onMenuClicked);

    connect(sidebar, &SidebarMenu::libraryClicked,     this, &MainWindow::onNavigateToLibrary);
    connect(sidebar, &SidebarMenu::favoritesClicked,   this, &MainWindow::onNavigateToFavorites);
    connect(sidebar, &SidebarMenu::collectionsClicked, this, &MainWindow::onNavigateToCollections);
    connect(sidebar, &SidebarMenu::settingsClicked,    this, &MainWindow::onNavigateToSettings);
    connect(sidebar, &SidebarMenu::termsClicked,       this, &MainWindow::onNavigateToTerms);
    connect(sidebar, &SidebarMenu::aboutClicked,       this, &MainWindow::onNavigateToAbout);
}

void MainWindow::applyTheme()
{
    QPalette palette;
    palette.setColor(QPalette::Window,     QColor(30, 30, 30));
    palette.setColor(QPalette::WindowText, Qt::white);
    setAutoFillBackground(true);
    setPalette(palette);
}

bool MainWindow::event(QEvent* event)
{
    if (event->type() == QEvent::TouchBegin  ||
        event->type() == QEvent::TouchUpdate ||
        event->type() == QEvent::TouchEnd)
    {
        handleTouchEvent(static_cast<QTouchEvent*>(event));
        return true;
    }
    return QMainWindow::event(event);
}

void MainWindow::handleTouchEvent(QTouchEvent* touchEvent)
{
    if (touchEvent->type() == QEvent::TouchBegin) {
        if (touchEvent->touchPoints().size() == 1) {
            QPoint pos = touchEvent->touchPoints().first().pos().toPoint();
            if (pos.x() < 50 && pos.y() < 50)
                toggleSidebar();
        }
    }
}

void MainWindow::toggleSidebar()
{
    sidebarOpen = !sidebarOpen;
    sidebar->setVisible(sidebarOpen);
}

void MainWindow::onMenuClicked()    { toggleSidebar(); }

void MainWindow::onNavigateToLibrary()
{
    screenStack->setCurrentWidget(libraryScreen.get());
    libraryScreen->showAllBooks();
    sidebarOpen = false;
    sidebar->hide();
}

void MainWindow::onNavigateToFavorites()
{
    screenStack->setCurrentWidget(libraryScreen.get());
    libraryScreen->showFavorites();
    sidebarOpen = false;
    sidebar->hide();
}

void MainWindow::onNavigateToCollections()
{
    // Coleções: por agora navega para biblioteca
    // (CollectionScreen pode ser adicionada futuramente)
    screenStack->setCurrentWidget(libraryScreen.get());
    libraryScreen->showAllBooks();
    sidebarOpen = false;
    sidebar->hide();
}

void MainWindow::onNavigateToSettings()
{
    screenStack->setCurrentWidget(settingsScreen.get());
    sidebarOpen = false;
    sidebar->hide();
}

void MainWindow::onNavigateToTerms()
{
    // Se TermsScreen existir no projeto, adicione aqui.
    // Por ora navega para About como fallback.
    screenStack->setCurrentWidget(aboutScreen.get());
    sidebarOpen = false;
    sidebar->hide();
}

void MainWindow::onNavigateToAbout()
{
    screenStack->setCurrentWidget(aboutScreen.get());
    sidebarOpen = false;
    sidebar->hide();
}

void MainWindow::onOpenBook(const QString& filePath)
{
    // ── FIX: garante que sidebar some antes de abrir o livro ──────────────
    sidebarOpen = false;
    sidebar->hide();

    readerScreen->openBook(filePath);
    screenStack->setCurrentWidget(readerScreen.get());
}

void MainWindow::onCloseReader()
{
    readerScreen->closeBook();
    screenStack->setCurrentWidget(libraryScreen.get());
}
