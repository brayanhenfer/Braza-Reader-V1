#pragma once

#include <QString>
#include <QStringList>

class LibraryManager
{
public:
    LibraryManager();
    ~LibraryManager();

    QStringList scanLibrary() const;
    bool addBook(const QString& filePath);
    bool removeBook(const QString& filePath);
    QString getLibraryPath() const;

private:
    void ensureDirectoryExists() const;
    QString libraryPath;
};