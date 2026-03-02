#pragma once
#include <QString>
#include <QColor>
#include <QSettings>
#include <memory>

class SettingsManager
{
public:
    SettingsManager();
    ~SettingsManager();

    QColor getMenuColor() const;
    void   setMenuColor(const QColor& color);

    QColor getBgColor() const;
    void   setBgColor(const QColor& color);

    bool getNightMode() const;
    void setNightMode(bool enabled);

    bool getSepiaEnabled() const;
    void setSepiaEnabled(bool enabled);

    int  getAmberIntensity() const;      // 0–100
    void setAmberIntensity(int value);

    int  getBrightness() const;          // 20–100
    void setBrightness(int value);

    int  getFontSize() const;
    void setFontSize(int size);

    void resetToDefaults();

private:
    std::unique_ptr<QSettings> settings;
};
