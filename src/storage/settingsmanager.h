#pragma once
#include <QString>
#include <QColor>
#include <QSettings>
#include <memory>

class SettingsManager {
public:
    SettingsManager();
    ~SettingsManager();

    QColor getMenuColor() const;
    void   setMenuColor(const QColor& color);

    // Cor de fundo do app (painel/grade)
    QColor getBgColor() const;
    void   setBgColor(const QColor& color);

    // Alias mantido para compatibilidade interna
    QColor getAppBgColor() const;
    void   setAppBgColor(const QColor& color);

    // Cor de janela (cards/painéis internos)
    QColor getWindowColor() const;
    void   setWindowColor(const QColor& color);

    // Brilho (0-100)
    int  getBrightness() const;
    void setBrightness(int v);

    bool getNightMode() const;
    void setNightMode(bool enabled);

    int  getAmberIntensity() const;
    void setAmberIntensity(int v);

    bool getSepiaEnabled() const;
    void setSepiaEnabled(bool e);

    void resetToDefaults();

private:
    std::unique_ptr<QSettings> settings;
};
