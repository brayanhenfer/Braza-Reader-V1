#include "mainwindow.h"
#include "sidebarmenu.h"
#include "libraryscreen.h"
#include "readerscreen.h"
#include "settingsscreen.h"
#include "aboutscreen.h"
#include "termsscreen.h"
#include "../storage/settingsmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTouchEvent>
#include <QPalette>
#include <QEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , sidebarOpen(false)
    , settingsManager(std::make_unique<SettingsManager>())
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
    sidebar->setMaximumWidth(250);
    sidebar->hide();

    screenStack = new QStackedWidget(this);

    libraryScreen  = std::make_unique<LibraryScreen>();
    readerScreen   = std::make_unique<ReaderScreen>();
    settingsScreen = std::make_unique<SettingsScreen>();
    aboutScreen    = std::make_unique<AboutScreen>();
    termsScreen    = std::make_unique<TermsScreen>();

    screenStack->addWidget(libraryScreen.get());
    screenStack->addWidget(readerScreen.get());
    screenStack->addWidget(settingsScreen.get());
    screenStack->addWidget(aboutScreen.get());
    screenStack->addWidget(termsScreen.get());

    screenStack->setCurrentWidget(libraryScreen.get());

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(screenStack, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::connectSignals()
{
    connect(libraryScreen.get(),  &LibraryScreen::menuClicked,    this, &MainWindow::onMenuClicked);
    connect(libraryScreen.get(),  &LibraryScreen::bookOpened,     this, &MainWindow::onOpenBook);
    connect(readerScreen.get(),   &ReaderScreen::backClicked,     this, &MainWindow::onCloseReader);
    connect(settingsScreen.get(), &SettingsScreen::menuClicked,   this, &MainWindow::onMenuClicked);
    connect(settingsScreen.get(), &SettingsScreen::settingsChanged, this, &MainWindow::onSettingsChanged);
    connect(aboutScreen.get(),    &AboutScreen::menuClicked,      this, &MainWindow::onMenuClicked);
    connect(termsScreen.get(),    &TermsScreen::backClicked,      this, &MainWindow::onNavigateToAbout);

    connect(sidebar, &SidebarMenu::libraryClicked,   this, &MainWindow::onNavigateToLibrary);
    connect(sidebar, &SidebarMenu::favoritesClicked, this, &MainWindow::onNavigateToFavorites);
    connect(sidebar, &SidebarMenu::settingsClicked,  this, &MainWindow::onNavigateToSettings);
    connect(sidebar, &SidebarMenu::termsClicked,     this, &MainWindow::onNavigateToTerms);
    connect(sidebar, &SidebarMenu::aboutClicked,     this, &MainWindow::onNavigateToAbout);
}

// ── Tema ─────────────────────────────────────────────────────────────────────

void MainWindow::applyTheme()
{
    QColor menuColor = settingsManager->getMenuColor();
    bool   nightMode = settingsManager->getNightMode();

    // Fundo geral
    QColor bg = nightMode ? QColor(15, 15, 15) : QColor(30, 30, 30);
    QPalette palette;
    palette.setColor(QPalette::Window,     bg);
    palette.setColor(QPalette::WindowText, Qt::white);
    setAutoFillBackground(true);
    setPalette(palette);

    applyMenuColor(menuColor);
}

// FIX PRINCIPAL: aplica a cor do menu em TODOS os widgets que têm topbar
void MainWindow::applyMenuColor(const QColor& color)
{
    QString css = QString("background-color: %1;").arg(color.name());

    // Sidebar
    sidebar->applyMenuColor(color);

    // Topbar de cada tela — cada Screen expõe setMenuColor()
    libraryScreen->setMenuColor(color);
    readerScreen->setMenuColor(color);
    settingsScreen->setMenuColor(color);
    aboutScreen->setMenuColor(color);
    termsScreen->setMenuColor(color);
}

// Slot chamado quando o usuário altera qualquer configuração
void MainWindow::onSettingsChanged()
{
    applyTheme();
}

// ── Eventos ──────────────────────────────────────────────────────────────────

bool MainWindow::event(QEvent* event)
{
    if (event->type() == QEvent::TouchBegin  ||
        event->type() == QEvent::TouchUpdate ||
        event->type() == QEvent::TouchEnd)
    {
        QTouchEvent* touchEvent = static_cast<QTouchEvent*>(event);
        handleTouchEvent(touchEvent);
        return true;
    }
    return QMainWindow::event(event);
}

void MainWindow::handleTouchEvent(QTouchEvent* touchEvent)
{
    if (touchEvent->type() == QEvent::TouchBegin) {
        if (touchEvent->touchPoints().size() == 1) {
            const QTouchEvent::TouchPoint& tp = touchEvent->touchPoints().first();
            QPoint pos = tp.pos().toPoint();
            if (pos.x() < 50 && pos.y() < 50) {
                toggleSidebar();
            }
        }
    }
}

void MainWindow::toggleSidebar()
{
    sidebarOpen = !sidebarOpen;
    if (sidebarOpen) sidebar->show();
    else             sidebar->hide();
}

// ── Navegação ────────────────────────────────────────────────────────────────

void MainWindow::onMenuClicked()  { toggleSidebar(); }

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

void MainWindow::onNavigateToSettings()
{
    screenStack->setCurrentWidget(settingsScreen.get());
    sidebarOpen = false;
    sidebar->hide();
}

void MainWindow::onNavigateToTerms()
{
    screenStack->setCurrentWidget(termsScreen.get());
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
    readerScreen->openBook(filePath);
    screenStack->setCurrentWidget(readerScreen.get());
}

void MainWindow::onCloseReader()
{
    readerScreen->closeBook();
    screenStack->setCurrentWidget(libraryScreen.get());
}
