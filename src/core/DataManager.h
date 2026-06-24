#pragma once

#include <QList>
#include <QString>
#include <QStringList>

#include "models/LogEntry.h"

class DataManager {
public:
    static QStringList getTrains();
    static QStringList getWagons(const QString &trainId);
    static QList<LogEntry> loadLogData(const QString &trainId, const QString &wagonId);
    static QStringList checkWagonIssues(const QString &trainId, const QString &wagonId);

private:
    static QString logsDirPath();
};
