#include "settingsmanager.h"
#include <QStandardPaths>

SettingsManager::SettingsManager()
    : settings(std::make_unique<QSettings>(
          QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/brazareader/settings.ini",
          QSettings::IniFormat)) {}

SettingsManager::~SettingsManager() = default;

QColor SettingsManager::getMenuColor() const {
    QColor c(settings->value("ui/menu_color","#1e6432").toString());
    return c.isValid() ? c : QColor(30,100,50);
}
void SettingsManager::setMenuColor(const QColor& c) {
    settings->setValue("ui/menu_color",c.name()); settings->sync(); }

// getBgColor / setBgColor — cor de fundo do app
QColor SettingsManager::getBgColor() const {
    QColor c(settings->value("ui/app_bg","#2b2b2b").toString());
    return c.isValid() ? c : QColor(43,43,43);
}
void SettingsManager::setBgColor(const QColor& c) {
    settings->setValue("ui/app_bg",c.name()); settings->sync(); }

// getAppBgColor / setAppBgColor — alias para compatibilidade
QColor SettingsManager::getAppBgColor() const { return getBgColor(); }
void   SettingsManager::setAppBgColor(const QColor& c) { setBgColor(c); }

// getWindowColor / setWindowColor — cor dos cards/painéis internos
QColor SettingsManager::getWindowColor() const {
    QColor c(settings->value("ui/window_color","#3a3a3a").toString());
    return c.isValid() ? c : QColor(58,58,58);
}
void SettingsManager::setWindowColor(const QColor& c) {
    settings->setValue("ui/window_color",c.name()); settings->sync(); }

// getBrightness / setBrightness (20-100)
int  SettingsManager::getBrightness() const { return settings->value("ui/brightness",100).toInt(); }
void SettingsManager::setBrightness(int v)  { settings->setValue("ui/brightness",v); settings->sync(); }

bool SettingsManager::getNightMode() const { return settings->value("ui/night_mode",false).toBool(); }
void SettingsManager::setNightMode(bool e) { settings->setValue("ui/night_mode",e); settings->sync(); }

int  SettingsManager::getAmberIntensity() const { return settings->value("ui/amber",0).toInt(); }
void SettingsManager::setAmberIntensity(int v) { settings->setValue("ui/amber",v); settings->sync(); }

bool SettingsManager::getSepiaEnabled() const { return settings->value("ui/sepia",false).toBool(); }
void SettingsManager::setSepiaEnabled(bool e) { settings->setValue("ui/sepia",e); settings->sync(); }

void SettingsManager::resetToDefaults() { settings->clear(); settings->sync(); }
