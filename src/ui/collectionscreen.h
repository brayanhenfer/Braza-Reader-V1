#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <memory>

class CollectionManager;

class CollectionScreen : public QWidget {
    Q_OBJECT
public:
    explicit CollectionScreen(QWidget* parent = nullptr);
    ~CollectionScreen();
    void loadCollections();
signals:
    void menuClicked();
protected:
    void showEvent(QShowEvent* event) override;
private slots:
    void onCreateCollection();
    void onDeleteSelected();
private:
    void setupUI();
    QListWidget* listWidget = nullptr;
    std::unique_ptr<CollectionManager> collManager;
};
