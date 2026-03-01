#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <memory>

class SettingsManager;

class SettingsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsScreen(QWidget* parent = nullptr);
    ~SettingsScreen();

signals:
    void menuClicked();
    void settingsChanged();

private slots:
    void onColorSelected(const QColor& color);
    void onNightModeToggled(bool enabled);
    void onResetDefaults();

private:
    void setupUI();
    void loadCurrentSettings();

    QVBoxLayout* mainLayout;
    QWidget* topBar;
    QPushButton* menuButton;
    QLabel* titleLabel;

    QPushButton* colorButton;
    QCheckBox* nightModeCheck;
    QPushButton* resetButton;
    QLabel* currentColorLabel;

    std::unique_ptr<SettingsManager> settingsManager;
};