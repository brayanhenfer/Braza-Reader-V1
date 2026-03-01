#include "librarymanager.h"
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QCoreApplication>

LibraryManager::LibraryManager()
{
    // Pega o diretório do executável: /home/ub/brv1/build/bin
    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);
    dir.cdUp(); // sobe para /home/ub/brv1/build
    dir.cdUp(); // sobe para /home/ub/brv1
    libraryPath = dir.absolutePath() + "/library";

    ensureDirectoryExists();
}

LibraryManager::~LibraryManager() = default;

void LibraryManager::ensureDirectoryExists() const
{
    QDir dir(libraryPath);
    if (!dir.exists()) {
        dir.mkpath(libraryPath);
    }
}

QStringList LibraryManager::scanLibrary() const
{
    QStringList pdfFiles;
    QDir dir(libraryPath);

    if (!dir.exists()) return pdfFiles;

    QStringList filters;
    filters << "*.pdf" << "*.PDF";

    QFileInfoList entries = dir.entryInfoList(filters, QDir::Files, QDir::Name | QDir::IgnoreCase);

    for (const QFileInfo& entry : entries) {
        pdfFiles.append(entry.absoluteFilePath());
    }

    return pdfFiles;
}

bool LibraryManager::addBook(const QString& filePath)
{
    QFileInfo source(filePath);
    if (!source.exists() || source.suffix().toLower() != "pdf") {
        return false;
    }

    QString destPath = libraryPath + "/" + source.fileName();
    if (QFile::exists(destPath)) {
        return true;
    }

    return QFile::copy(filePath, destPath);
}

bool LibraryManager::removeBook(const QString& filePath)
{
    QFile file(filePath);
    if (file.exists()) {
        return file.remove();
    }
    return false;
}

QString LibraryManager::getLibraryPath() const
{
    return libraryPath;
}