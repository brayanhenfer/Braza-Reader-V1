#include "mainwindow.h"
#include "sidebarmenu.h"
#include "libraryscreen.h"
#include "readerscreen.h"
#include "settingsscreen.h"
#include "aboutscreen.h"
#include "termsscreen.h"
#include "collectionscreen.h"
#include "../storage/settingsmanager.h"

#include <QHBoxLayout>
#include <QTouchEvent>
#include <QPalette>
#include <QEvent>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
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
    QHBoxLayout* lay = new QHBoxLayout(centralWidget);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    sidebar = new SidebarMenu(this);
    sidebar->setMaximumWidth(250);
    sidebar->hide();

    screenStack = new QStackedWidget(this);

    libraryScreen    = std::make_unique<LibraryScreen>();
    readerScreen     = std::make_unique<ReaderScreen>();
    settingsScreen   = std::make_unique<SettingsScreen>();
    aboutScreen      = std::make_unique<AboutScreen>();
    termsScreen      = std::make_unique<TermsScreen>();
    collectionScreen = std::make_unique<CollectionScreen>();

    screenStack->addWidget(libraryScreen.get());
    screenStack->addWidget(readerScreen.get());
    screenStack->addWidget(settingsScreen.get());
    screenStack->addWidget(aboutScreen.get());
    screenStack->addWidget(termsScreen.get());
    screenStack->addWidget(collectionScreen.get());
    screenStack->setCurrentWidget(libraryScreen.get());

    lay->addWidget(sidebar);
    lay->addWidget(screenStack, 1);
    setCentralWidget(centralWidget);
}

void MainWindow::connectSignals()
{
    connect(libraryScreen.get(),    &LibraryScreen::menuClicked,      this, &MainWindow::onMenuClicked);
    connect(libraryScreen.get(),    &LibraryScreen::bookOpened,       this, &MainWindow::onOpenBook);
    connect(collectionScreen.get(), &CollectionScreen::menuClicked,   this, &MainWindow::onMenuClicked);
    connect(collectionScreen.get(), &CollectionScreen::bookOpened,    this, &MainWindow::onOpenBook);
    connect(readerScreen.get(),     &ReaderScreen::backClicked,       this, &MainWindow::onCloseReader);
    connect(settingsScreen.get(),   &SettingsScreen::menuClicked,     this, &MainWindow::onMenuClicked);
    connect(settingsScreen.get(),   &SettingsScreen::settingsChanged, this, &MainWindow::onSettingsChanged);
    connect(aboutScreen.get(),      &AboutScreen::menuClicked,        this, &MainWindow::onMenuClicked);

    // FIX: termos volta à biblioteca, não à "sobre"
    connect(termsScreen.get(),      &TermsScreen::backClicked,        this, &MainWindow::onNavigateToLibrary);

    connect(sidebar, &SidebarMenu::libraryClicked,     this, &MainWindow::onNavigateToLibrary);
    connect(sidebar, &SidebarMenu::favoritesClicked,   this, &MainWindow::onNavigateToFavorites);
    connect(sidebar, &SidebarMenu::collectionsClicked, this, &MainWindow::onNavigateToCollections);
    connect(sidebar, &SidebarMenu::settingsClicked,    this, &MainWindow::onNavigateToSettings);
    connect(sidebar, &SidebarMenu::termsClicked,       this, &MainWindow::onNavigateToTerms);
    connect(sidebar, &SidebarMenu::aboutClicked,       this, &MainWindow::onNavigateToAbout);
}

// ── Tema completo ─────────────────────────────────────────────────────────────

void MainWindow::applyTheme()
{
    QColor menuColor  = settingsManager->getMenuColor();
    // FIX: bgColor é a cor de fundo do APP (cinza escuro por padrão, não a cor de janelas)
    QColor bgColor    = settingsManager->getBgColor();      // cor do fundo externo (tela toda)
    QColor winColor   = settingsManager->getWindowColor();  // cor das janelas/cards internas
    bool   night      = settingsManager->getNightMode();
    bool   sepia      = settingsManager->getSepiaEnabled();
    int    amber      = settingsManager->getAmberIntensity();
    int    brightness = settingsManager->getBrightness();

    // Cor de fundo do app (o que aparece "atrás" de tudo)
    QString appBg = night ? "#080808" : bgColor.name();
    centralWidget->setStyleSheet(QString("background:%1;").arg(appBg));

    // Propaga cor do menu para todas as telas
    sidebar->applyMenuColor(menuColor);
    libraryScreen->setMenuColor(menuColor);
    readerScreen->setMenuColor(menuColor);
    settingsScreen->setMenuColor(menuColor);
    aboutScreen->setMenuColor(menuColor);
    termsScreen->setMenuColor(menuColor);

    // FIX: cor das janelas/cards internas (ex: grade da biblioteca)
    libraryScreen->setWindowColor(winColor);
    collectionScreen->setWindowColor(winColor);

    // Modo noturno
    readerScreen->applyNightMode(night);

    // Filtros visuais: âmbar e sépia no leitor
    readerScreen->setAmberIntensity(amber);
    readerScreen->setSepiaEnabled(sepia);

    // Brilho via overlay preto semitransparente
    if (!brightnessOverlay) {
        brightnessOverlay = new QWidget(this);
        brightnessOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    brightnessOverlay->setGeometry(rect());
    if (brightness < 100) {
        int alpha = qBound(0, qRound((100 - brightness) * 2.2), 200);
        brightnessOverlay->setStyleSheet(
            QString("background:rgba(0,0,0,%1);").arg(alpha));
        brightnessOverlay->raise();
        brightnessOverlay->show();
    } else {
        brightnessOverlay->hide();
    }
}

void MainWindow::onSettingsChanged() { applyTheme(); }

// ── Eventos ──────────────────────────────────────────────────────────────────

bool MainWindow::event(QEvent* ev)
{
    if (ev->type() == QEvent::TouchBegin  ||
        ev->type() == QEvent::TouchUpdate ||
        ev->type() == QEvent::TouchEnd)
    {
        handleTouchEvent(static_cast<QTouchEvent*>(ev));
        return true;
    }
    return QMainWindow::event(ev);
}

void MainWindow::handleTouchEvent(QTouchEvent* te)
{
    if (te->type() == QEvent::TouchBegin && te->touchPoints().size() == 1) {
        QPoint pos = te->touchPoints().first().pos().toPoint();
        if (pos.x() < 50 && pos.y() < 50) toggleSidebar();
    }
}

void MainWindow::toggleSidebar()
{
    sidebarOpen = !sidebarOpen;
    if (sidebarOpen) sidebar->show(); else sidebar->hide();
}

void MainWindow::closeSidebar()
{
    sidebarOpen = false;
    sidebar->hide();
}

// ── Navegação ────────────────────────────────────────────────────────────────

void MainWindow::onMenuClicked() { toggleSidebar(); }

void MainWindow::onNavigateToLibrary()
{
    screenStack->setCurrentWidget(libraryScreen.get());
    libraryScreen->showAllBooks();
    closeSidebar();
}

void MainWindow::onNavigateToFavorites()
{
    screenStack->setCurrentWidget(libraryScreen.get());
    libraryScreen->showFavorites();   // FIX: navega para biblioteca em modo favoritos
    closeSidebar();
}

void MainWindow::onNavigateToCollections()
{
    screenStack->setCurrentWidget(collectionScreen.get());
    collectionScreen->refresh();
    closeSidebar();
}

void MainWindow::onNavigateToSettings()
{
    screenStack->setCurrentWidget(settingsScreen.get());
    closeSidebar();
}

void MainWindow::onNavigateToTerms()
{
    screenStack->setCurrentWidget(termsScreen.get());
    closeSidebar();
}

void MainWindow::onNavigateToAbout()
{
    screenStack->setCurrentWidget(aboutScreen.get());
    closeSidebar();
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
