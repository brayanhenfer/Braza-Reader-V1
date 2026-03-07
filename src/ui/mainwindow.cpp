#include "mainwindow.h"
#include "sidebarmenu.h"
#include "libraryscreen.h"
#include "readerscreen.h"
#include "settingsscreen.h"
#include "aboutscreen.h"
#include "collectionscreen.h"
#include "termsscreen.h"
#include "../storage/settingsmanager.h"
#include <QHBoxLayout>
#include <QTouchEvent>
#include <QPalette>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget* parent):QMainWindow(parent),sidebarOpen(false){
    setAttribute(Qt::WA_AcceptTouchEvents);
    setWindowState(Qt::WindowFullScreen);
    setupUI(); connectSignals(); applyTheme();
    libraryScreen->loadLibrary();
    onApplySettings();
}
MainWindow::~MainWindow()=default;

void MainWindow::setupUI(){
    centralWidget=new QWidget(this);
    auto* mainLayout=new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0,0,0,0); mainLayout->setSpacing(0);
    sidebar=new SidebarMenu(this); sidebar->setMaximumWidth(230); sidebar->hide();
    screenStack=new QStackedWidget(this);
    libraryScreen    =std::make_unique<LibraryScreen>();
    readerScreen     =std::make_unique<ReaderScreen>();
    settingsScreen   =std::make_unique<SettingsScreen>();
    aboutScreen      =std::make_unique<AboutScreen>();
    collectionScreen =std::make_unique<CollectionScreen>();
    termsScreen      =std::make_unique<TermsScreen>();
    for(auto* w:{(QWidget*)libraryScreen.get(),(QWidget*)readerScreen.get(),
                 (QWidget*)settingsScreen.get(),(QWidget*)aboutScreen.get(),
                 (QWidget*)collectionScreen.get(),(QWidget*)termsScreen.get()})
        screenStack->addWidget(w);
    screenStack->setCurrentWidget(libraryScreen.get());
    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(screenStack,1);
    setCentralWidget(centralWidget);

    // Overlay de brilho — cobre toda a janela, ignora eventos de mouse
    brightnessOverlay = new QWidget(this);
    brightnessOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    brightnessOverlay->hide();
    brightnessOverlay->raise();
}

void MainWindow::connectSignals(){
    connect(libraryScreen.get(),    &LibraryScreen::menuClicked,    this, &MainWindow::onMenuClicked);
    connect(libraryScreen.get(),    &LibraryScreen::bookOpened,     this, &MainWindow::onOpenBook);
    connect(readerScreen.get(),     &ReaderScreen::backClicked,     this, &MainWindow::onCloseReader);
    connect(settingsScreen.get(),   &SettingsScreen::menuClicked,   this, &MainWindow::onMenuClicked);
    connect(aboutScreen.get(),      &AboutScreen::menuClicked,      this, &MainWindow::onMenuClicked);
    connect(collectionScreen.get(), &CollectionScreen::menuClicked,  this, &MainWindow::onMenuClicked);
    connect(collectionScreen.get(), &CollectionScreen::backClicked,  this, &MainWindow::onNavigateToLibrary);
    connect(collectionScreen.get(), &CollectionScreen::bookOpened,   this, &MainWindow::onOpenBook);
    connect(termsScreen.get(),      &TermsScreen::menuClicked,      this, &MainWindow::onMenuClicked);
    connect(sidebar, &SidebarMenu::libraryClicked,     this, &MainWindow::onNavigateToLibrary);
    connect(sidebar, &SidebarMenu::favoritesClicked,   this, &MainWindow::onNavigateToFavorites);
    connect(sidebar, &SidebarMenu::collectionsClicked, this, &MainWindow::onNavigateToCollections);
    connect(sidebar, &SidebarMenu::settingsClicked,    this, &MainWindow::onNavigateToSettings);
    connect(sidebar, &SidebarMenu::termsClicked,       this, &MainWindow::onNavigateToTerms);
    connect(sidebar, &SidebarMenu::aboutClicked,       this, &MainWindow::onNavigateToAbout);

    connect(settingsScreen.get(), &SettingsScreen::settingsChanged,
            this, &MainWindow::onApplySettings);
}

void MainWindow::applyTheme(){
    QPalette p; p.setColor(QPalette::Window,QColor(30,30,30));
    setAutoFillBackground(true); setPalette(p);
}

void MainWindow::onApplySettings(){
    SettingsManager sm;
    readerScreen->setAmberIntensity(sm.getAmberIntensity());
    readerScreen->applyNightMode(sm.getNightMode());
    readerScreen->setSepiaEnabled(sm.getSepiaEnabled());
    readerScreen->setMenuColor(sm.getMenuColor());
    readerScreen->setAppBgColor(sm.getAppBgColor());
    settingsScreen->setMenuColor(sm.getMenuColor());
    sidebar->setStyleSheet(QString("background:%1;").arg(sm.getMenuColor().name()));
    applyBrightness(sm.getBrightness());
}

void MainWindow::applyBrightness(int percent){
    if(!brightnessOverlay) return;
    const int clamped = qBound(20, percent, 100);
    if(clamped >= 100){
        brightnessOverlay->hide();
        return;
    }
    // 20→alpha≈176   99→alpha≈2  — overlay preto semi-transparente
    const int alpha = qBound(2, int((100 - clamped) * 2.0), 200);
    brightnessOverlay->setStyleSheet(
        QString("background-color:rgba(0,0,0,%1);").arg(alpha));
    brightnessOverlay->setGeometry(0, 0, width(), height());
    brightnessOverlay->show();
    brightnessOverlay->raise();
}

void MainWindow::resizeEvent(QResizeEvent* e){
    QMainWindow::resizeEvent(e);
    if(brightnessOverlay && brightnessOverlay->isVisible())
        brightnessOverlay->setGeometry(0, 0, width(), height());
}

bool MainWindow::event(QEvent* event){
    if(event->type()==QEvent::TouchBegin){
        auto* te=static_cast<QTouchEvent*>(event);
        if(te->touchPoints().size()==1){
            QPoint pos=te->touchPoints().first().pos().toPoint();
            if(pos.x()<50&&pos.y()<50){ toggleSidebar(); return true; }
        }
    }
    return QMainWindow::event(event);
}
void MainWindow::handleTouchEvent(QTouchEvent*){}
void MainWindow::toggleSidebar(){ sidebarOpen=!sidebarOpen; sidebar->setVisible(sidebarOpen); }
void MainWindow::onMenuClicked(){ toggleSidebar(); }

void MainWindow::onNavigateToLibrary()  { screenStack->setCurrentWidget(libraryScreen.get());    libraryScreen->showAllBooks();  sidebarOpen=false; sidebar->hide(); }
void MainWindow::onNavigateToFavorites(){ screenStack->setCurrentWidget(libraryScreen.get());    libraryScreen->showFavorites(); sidebarOpen=false; sidebar->hide(); }
void MainWindow::onNavigateToCollections(){ screenStack->setCurrentWidget(collectionScreen.get()); sidebarOpen=false; sidebar->hide(); }
void MainWindow::onNavigateToSettings() { screenStack->setCurrentWidget(settingsScreen.get());   sidebarOpen=false; sidebar->hide(); }
void MainWindow::onNavigateToTerms()    { screenStack->setCurrentWidget(termsScreen.get());      sidebarOpen=false; sidebar->hide(); }
void MainWindow::onNavigateToAbout()    { screenStack->setCurrentWidget(aboutScreen.get());      sidebarOpen=false; sidebar->hide(); }

void MainWindow::onOpenBook(const QString& filePath){
    sidebarOpen=false; sidebar->hide();
    readerScreen->openBook(filePath);
    screenStack->setCurrentWidget(readerScreen.get());
}
void MainWindow::onCloseReader(){
    readerScreen->closeBook();
    screenStack->setCurrentWidget(libraryScreen.get());
    libraryScreen->loadLibrary();  // atualiza % de leitura na volta
}
