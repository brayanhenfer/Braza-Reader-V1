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
#include <QGuiApplication>
#include <QScreen>

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
    connect(libraryScreen.get(),  &LibraryScreen::menuClicked,      this, &MainWindow::onMenuClicked);
    connect(libraryScreen.get(),  &LibraryScreen::bookOpened,       this, &MainWindow::onOpenBook);
    connect(readerScreen.get(),   &ReaderScreen::backClicked,       this, &MainWindow::onCloseReader);
    connect(settingsScreen.get(), &SettingsScreen::menuClicked,     this, &MainWindow::onMenuClicked);
    connect(settingsScreen.get(), &SettingsScreen::settingsChanged, this, &MainWindow::onSettingsChanged);
    connect(aboutScreen.get(),    &AboutScreen::menuClicked,        this, &MainWindow::onMenuClicked);
    connect(termsScreen.get(),    &TermsScreen::backClicked,        this, &MainWindow::onNavigateToAbout);

    connect(sidebar, &SidebarMenu::libraryClicked,   this, &MainWindow::onNavigateToLibrary);
    connect(sidebar, &SidebarMenu::favoritesClicked, this, &MainWindow::onNavigateToFavorites);
    connect(sidebar, &SidebarMenu::settingsClicked,  this, &MainWindow::onNavigateToSettings);
    connect(sidebar, &SidebarMenu::termsClicked,     this, &MainWindow::onNavigateToTerms);
    connect(sidebar, &SidebarMenu::aboutClicked,     this, &MainWindow::onNavigateToAbout);
}

// ── Tema completo ─────────────────────────────────────────────────────────────

void MainWindow::applyTheme()
{
    QColor menuColor = settingsManager->getMenuColor();
    QColor bgColor   = settingsManager->getBgColor();
    bool   night     = settingsManager->getNightMode();
    bool   sepia     = settingsManager->getSepiaEnabled();
    int    amber     = settingsManager->getAmberIntensity();
    int    brightness= settingsManager->getBrightness();

    // FIX: cor de fundo real aplicada via setStyleSheet no centralWidget
    QString bgCss = night ? "#0a0a0a" : bgColor.name();
    centralWidget->setStyleSheet(QString("background:%1;").arg(bgCss));

    // Propagação de cor do menu
    sidebar->applyMenuColor(menuColor);
    libraryScreen->setMenuColor(menuColor);
    readerScreen->setMenuColor(menuColor);
    settingsScreen->setMenuColor(menuColor);
    aboutScreen->setMenuColor(menuColor);
    termsScreen->setMenuColor(menuColor);

    // Modo noturno no leitor
    readerScreen->applyNightMode(night);

    // Brilho: overlay escuro semitransparente sobre tudo (simulação sem acesso ao backlight)
    // O RPi com X11 pode controlar brilho real via xrandr:
    //   system("xrandr --output HDMI-1 --brightness 0.7");
    // Aqui usamos overlay Qt como fallback universal.
    if (!brightnessOverlay) {
        brightnessOverlay = new QWidget(this);
        brightnessOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        brightnessOverlay->setStyleSheet("background:black;");
    }
    brightnessOverlay->setGeometry(rect());
    if (brightness < 100) {
        int alpha = qRound((100 - brightness) * 2.2); // 0–80% opacidade
        brightnessOverlay->setStyleSheet(QString("background:rgba(0,0,0,%1);").arg(alpha));
        brightnessOverlay->raise();
        brightnessOverlay->show();
    } else {
        brightnessOverlay->hide();
    }

    // Sépia e âmbar são aplicados no ReaderScreen via PageWidget::paintEvent
    // (passamos os valores ao abrir livro)
    Q_UNUSED(sepia); Q_UNUSED(amber);
}

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
        handleTouchEvent(static_cast<QTouchEvent*>(event));
        return true;
    }
    return QMainWindow::event(event);
}

void MainWindow::handleTouchEvent(QTouchEvent* touchEvent)
{
    if (touchEvent->type() == QEvent::TouchBegin &&
        touchEvent->touchPoints().size() == 1)
    {
        QPoint pos = touchEvent->touchPoints().first().pos().toPoint();
        if (pos.x() < 50 && pos.y() < 50) toggleSidebar();
    }
}

void MainWindow::toggleSidebar()
{
    sidebarOpen = !sidebarOpen;
    if (sidebarOpen) sidebar->show(); else sidebar->hide();
}

// ── Navegação ────────────────────────────────────────────────────────────────

void MainWindow::onMenuClicked() { toggleSidebar(); }

void MainWindow::onNavigateToLibrary()
{
    screenStack->setCurrentWidget(libraryScreen.get());
    libraryScreen->showAllBooks();
    sidebarOpen = false; sidebar->hide();
}

void MainWindow::onNavigateToFavorites()
{
    screenStack->setCurrentWidget(libraryScreen.get());
    libraryScreen->showFavorites();
    sidebarOpen = false; sidebar->hide();
}

void MainWindow::onNavigateToSettings()
{
    screenStack->setCurrentWidget(settingsScreen.get());
    sidebarOpen = false; sidebar->hide();
}

void MainWindow::onNavigateToTerms()
{
    screenStack->setCurrentWidget(termsScreen.get());
    sidebarOpen = false; sidebar->hide();
}

void MainWindow::onNavigateToAbout()
{
    screenStack->setCurrentWidget(aboutScreen.get());
    sidebarOpen = false; sidebar->hide();
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
