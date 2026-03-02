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

private:
    void setupUI();
    void loadCurrentSettings();

    QWidget*     topBar;
    QPushButton* menuBtn;

    // Previews de cor
    QLabel*      menuColorPreview = nullptr;
    QLabel*      bgColorPreview   = nullptr;
    QLabel*      winColorPreview  = nullptr;

    QSlider*     brightnessSlider;
    QLabel*      brightnessLabel;
    QSlider*     amberSlider;
    QLabel*      amberLabel;

    QCheckBox*   nightModeCheck;
    QCheckBox*   sepiaCheck;

    std::unique_ptr<SettingsManager> settingsManager;
};
