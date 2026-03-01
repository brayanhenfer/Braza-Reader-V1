#include "settingsmanager.h"
#include <QStandardPaths>

SettingsManager::SettingsManager()
    : settings(std::make_unique<QSettings>(
          QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/brazareader/settings.ini",
          QSettings::IniFormat))
{
}

SettingsManager::~SettingsManager() = default;

QColor SettingsManager::getMenuColor() const
{
    QString colorStr = settings->value("ui/menu_color", "#1e6432").toString();
    QColor color(colorStr);
    if (!color.isValid()) {
        color = QColor(30, 100, 50);
    }
    return color;
}

void SettingsManager::setMenuColor(const QColor& color)
{
    settings->setValue("ui/menu_color", color.name());
    settings->sync();
}

bool SettingsManager::getNightMode() const
{
    return settings->value("ui/night_mode", false).toBool();
}

void SettingsManager::setNightMode(bool enabled)
{
    settings->setValue("ui/night_mode", enabled);
    settings->sync();
}

int SettingsManager::getFontSize() const
{
    return settings->value("ui/font_size", 12).toInt();
}

void SettingsManager::setFontSize(int size)
{
    settings->setValue("ui/font_size", size);
    settings->sync();
}

void SettingsManager::resetToDefaults()
{
    settings->clear();
    settings->sync();
}
