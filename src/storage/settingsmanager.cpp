#include "settingsmanager.h"
#include <QStandardPaths>
#include <QDir>

SettingsManager::SettingsManager()
    : settings(std::make_unique<QSettings>(
          QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
          + "/brazareader/settings.ini", QSettings::IniFormat))
{}

SettingsManager::~SettingsManager() = default;

QColor SettingsManager::getMenuColor() const {
    QColor c(settings->value("ui/menu_color", "#1e6432").toString());
    return c.isValid() ? c : QColor("#1e6432");
}
void SettingsManager::setMenuColor(const QColor& c) {
    settings->setValue("ui/menu_color", c.name()); settings->sync();
}

// Cor de fundo do app (espaço externo, default: cinza escuro do app)
QColor SettingsManager::getBgColor() const {
    QColor c(settings->value("ui/bg_color", "#1a1a1a").toString());
    return c.isValid() ? c : QColor("#1a1a1a");
}
void SettingsManager::setBgColor(const QColor& c) {
    settings->setValue("ui/bg_color", c.name()); settings->sync();
}

// Cor de janela (cards/grades, default: cinza médio dos cards)
QColor SettingsManager::getWindowColor() const {
    QColor c(settings->value("ui/window_color", "#2b2b2b").toString());
    return c.isValid() ? c : QColor("#2b2b2b");
}
void SettingsManager::setWindowColor(const QColor& c) {
    settings->setValue("ui/window_color", c.name()); settings->sync();
}

bool SettingsManager::getNightMode() const {
    return settings->value("ui/night_mode", false).toBool();
}
void SettingsManager::setNightMode(bool e) {
    settings->setValue("ui/night_mode", e); settings->sync();
}

bool SettingsManager::getSepiaEnabled() const {
    return settings->value("ui/sepia", false).toBool();
}
void SettingsManager::setSepiaEnabled(bool e) {
    settings->setValue("ui/sepia", e); settings->sync();
}

int SettingsManager::getAmberIntensity() const {
    return settings->value("ui/amber", 0).toInt();
}
void SettingsManager::setAmberIntensity(int v) {
    settings->setValue("ui/amber", v); settings->sync();
}

int SettingsManager::getBrightness() const {
    return settings->value("ui/brightness", 100).toInt();
}
void SettingsManager::setBrightness(int v) {
    settings->setValue("ui/brightness", v); settings->sync();
}

int SettingsManager::getFontSize() const {
    return settings->value("ui/font_size", 12).toInt();
}
void SettingsManager::setFontSize(int s) {
    settings->setValue("ui/font_size", s); settings->sync();
}

void SettingsManager::resetToDefaults() {
    settings->clear(); settings->sync();
}
