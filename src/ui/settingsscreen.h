#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QColor>
#include <memory>

class SettingsManager;

class SettingsScreen : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsScreen(QWidget* parent = nullptr);
    ~SettingsScreen();

    void setMenuColor(const QColor& color);

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

    QWidget*     topBar;
    QPushButton* menuButton;
    QLabel*      titleLabel;

    QLabel*      currentColorLabel;
    QPushButton* colorButton;
    QLabel*      currentBgLabel;
    QPushButton* bgColorButton;

    QSlider*     brightnessSlider;
    QSlider*     amberSlider;

    QCheckBox*   nightModeCheck;
    QCheckBox*   sepiaCheck;

    QPushButton* resetButton;

    std::unique_ptr<SettingsManager> settingsManager;
};
