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

    // Cor do menu (topbar)
    QColor getMenuColor() const;
    void   setMenuColor(const QColor& c);

    // Cor de fundo do app (tela toda, "atrás" do conteúdo)
    QColor getBgColor() const;
    void   setBgColor(const QColor& c);

    // Cor de janela (cards, grades internas, painéis)
    QColor getWindowColor() const;
    void   setWindowColor(const QColor& c);

    bool getNightMode() const;
    void setNightMode(bool e);

    bool getSepiaEnabled() const;
    void setSepiaEnabled(bool e);

    int  getAmberIntensity() const;
    void setAmberIntensity(int v);

    int  getBrightness() const;
    void setBrightness(int v);

    int  getFontSize() const;
    void setFontSize(int s);

    void resetToDefaults();

private:
    std::unique_ptr<QSettings> settings;
};
