#pragma once

#include <QString>
#include <sqlite3.h>

class ProgressManager
{
public:
    ProgressManager();
    ~ProgressManager();

    int  getLastPage(const QString& bookTitle) const;
    void saveProgress(const QString& bookTitle, int pageNumber);
    void clearProgress(const QString& bookTitle);
    void renameTitle(const QString& oldTitle, const QString& newTitle);

private:
    void initializeDatabase();
    void createTableIfNeeded();
    QString getDatabasePath() const;

    sqlite3* database;
};