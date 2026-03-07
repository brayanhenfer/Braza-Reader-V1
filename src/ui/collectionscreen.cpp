#include "collectionscreen.h"
#include "topbar_helper.h"
#include "../storage/collectionmanager.h"
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QShowEvent>

CollectionScreen::CollectionScreen(QWidget* parent)
    : QWidget(parent)
    , collManager(std::make_unique<CollectionManager>())
{ setupUI(); loadCollections(); }
CollectionScreen::~CollectionScreen()=default;

void CollectionScreen::setupUI(){
    auto* mainLayout=new QVBoxLayout(this); mainLayout->setContentsMargins(0,0,0,0); mainLayout->setSpacing(0);

    auto* menuBtn=new QPushButton(QString::fromUtf8("☰"),this); menuBtn->setFixedSize(40,40);
    menuBtn->setStyleSheet("QPushButton{background:transparent;color:white;font-size:24px;border:none;}"
                           "QPushButton:pressed{background:rgba(255,255,255,0.2);border-radius:20px;}");
    connect(menuBtn,&QPushButton::clicked,this,&CollectionScreen::menuClicked);
    mainLayout->addWidget(TopBarHelper::create(this,menuBtn));

    // Toolbar
    auto* toolbar=new QWidget(this); toolbar->setFixedHeight(52);
    toolbar->setStyleSheet("background:#222;border-bottom:1px solid #333;");
    auto* tl=new QHBoxLayout(toolbar); tl->setContentsMargins(16,8,16,8); tl->setSpacing(10);

    auto* titleLbl=new QLabel("Minhas Coleções",toolbar);
    titleLbl->setStyleSheet("color:white;font-size:16px;font-weight:bold;");
    tl->addWidget(titleLbl);
    tl->addStretch();

    auto* addBtn=new QPushButton("＋ Nova Coleção",toolbar);
    addBtn->setStyleSheet("QPushButton{background:#1e6432;color:white;padding:6px 14px;border-radius:6px;font-size:13px;}"
                          "QPushButton:pressed{background:#2a8040;}");
    connect(addBtn,&QPushButton::clicked,this,&CollectionScreen::onCreateCollection);
    tl->addWidget(addBtn);

    auto* delBtn=new QPushButton("🗑 Excluir",toolbar);
    delBtn->setStyleSheet("QPushButton{background:#7a1010;color:white;padding:6px 14px;border-radius:6px;font-size:13px;}"
                          "QPushButton:pressed{background:#a01010;}");
    connect(delBtn,&QPushButton::clicked,this,&CollectionScreen::onDeleteSelected);
    tl->addWidget(delBtn);
    mainLayout->addWidget(toolbar);

    // Lista
    listWidget=new QListWidget(this);
    listWidget->setStyleSheet(
        "QListWidget{background:#2b2b2b;color:white;border:none;font-size:15px;}"
        "QListWidget::item{padding:16px 20px;border-bottom:1px solid #333;}"
        "QListWidget::item:selected{background:#1e5c28;}");
    mainLayout->addWidget(listWidget,1);
}

void CollectionScreen::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    loadCollections(); // sempre recarrega ao entrar na tela
}

void CollectionScreen::loadCollections(){
    if(!listWidget) return;
    listWidget->clear();
    const QStringList cols=collManager->getCollections();
    if(cols.isEmpty()){
        auto* item=new QListWidgetItem("  Nenhuma coleção ainda. Toque em '+ Nova Coleção' para criar.");
        item->setForeground(QColor("#666")); listWidget->addItem(item); return;
    }
    for(const QString& col:cols){
        const int count=collManager->getBooksInCollection(col).size();
        auto* item=new QListWidgetItem(
            QString("  📚  %1   (%2 livro%3)").arg(col).arg(count).arg(count!=1?"s":""));
        item->setData(Qt::UserRole,col); listWidget->addItem(item);
    }
}

void CollectionScreen::onCreateCollection(){
    const QString name=QInputDialog::getText(this,"Nova Coleção","Nome da coleção:").trimmed();
    if(!name.isEmpty()){ collManager->addCollection(name); loadCollections(); }
}

void CollectionScreen::onDeleteSelected(){
    if(!listWidget->currentItem()) return;
    const QString col=listWidget->currentItem()->data(Qt::UserRole).toString();
    if(col.isEmpty()) return;
    auto* mb=new QMessageBox(this);
    mb->setStyleSheet("background:#1a1a1a;color:white;");
    mb->setWindowTitle("Confirmar");
    mb->setText(QString("Excluir a coleção \"%1\"?").arg(col));
    mb->setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    if(mb->exec()==QMessageBox::Yes){ collManager->removeCollection(col); loadCollections(); }
    mb->deleteLater();
}
