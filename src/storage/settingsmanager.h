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
    void setMenuColor(const QColor& color);

    bool getNightMode() const;
    void setNightMode(bool enabled);

    int getFontSize() const;
    void setFontSize(int size);

    void resetToDefaults();

private:
    std::unique_ptr<QSettings> settings;
};
