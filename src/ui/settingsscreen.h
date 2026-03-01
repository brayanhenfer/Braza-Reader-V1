#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
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
    void onPresetSelected(const QString& menuColor, const QString& bgColor, bool nightMode);

private:
    void setupUI();
    void loadCurrentSettings();
    void applyPresetButton(QPushButton* btn, const QString& label,
                           const QString& menuColor, const QString& bgColor, bool nightMode);

    QVBoxLayout*  mainLayout;
    QWidget*      topBar;
    QPushButton*  menuButton;
    QLabel*       titleLabel;

    QPushButton*  colorButton;
    QPushButton*  bgColorButton;
    QCheckBox*    nightModeCheck;
    QPushButton*  resetButton;
    QLabel*       currentColorLabel;
    QLabel*       currentBgColorLabel;

    std::unique_ptr<SettingsManager> settingsManager;
};
