#include "core/DataManager.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringConverter>
#include <QTextStream>

namespace {

double parseDouble(const QString &value, double fallback = 0.0)
{
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    return ok ? parsed : fallback;
}

int parseInt(const QString &value, int fallback = 0)
{
    bool ok = false;
    const int parsed = value.toInt(&ok);
    return ok ? parsed : fallback;
}

QString formatIssueCode(const QString &code)
{
    return code.trimmed().toUpper();
}

} // namespace

QString DataManager::logsDirPath()
{
    return QDir::current().filePath(QStringLiteral("data_logs"));
}

QStringList DataManager::getTrains()
{
    QDir logsDir(logsDirPath());
    if (!logsDir.exists()) {
        return {};
    }

    QStringList trains;
    const QFileInfoList entries = logsDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QFileInfo &entry : entries) {
        const QString name = entry.fileName();
        if (name.startsWith(QStringLiteral("train_"))) {
            trains.append(name.mid(QStringLiteral("train_").size()));
        }
    }

    trains.sort();
    return trains;
}

QStringList DataManager::getWagons(const QString &trainId)
{
    QDir trainDir(QDir(logsDirPath()).filePath(QStringLiteral("train_%1").arg(trainId)));
    if (!trainDir.exists()) {
        return {};
    }

    QStringList wagons;
    const QFileInfoList entries = trainDir.entryInfoList({QStringLiteral("*.log")}, QDir::Files, QDir::Name);
    for (const QFileInfo &entry : entries) {
        wagons.append(entry.completeBaseName());
    }

    wagons.sort();
    return wagons;
}

QList<LogEntry> DataManager::loadLogData(const QString &trainId, const QString &wagonId)
{
    const QString filePath = QDir(logsDirPath())
                                 .filePath(QStringLiteral("train_%1/%2.log").arg(trainId, wagonId));

    QFile file(filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    QList<LogEntry> data;
    if (stream.atEnd()) {
        return data;
    }

    stream.readLine();

    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const QStringList columns = line.split(QLatin1Char(','));
        if (columns.size() < 9) {
            continue;
        }

        LogEntry entry;
        entry.time = columns.at(0).trimmed();
        entry.pipePressure = parseDouble(columns.at(1));
        entry.cylinderPressure = parseDouble(columns.at(2));
        entry.rod = parseInt(columns.at(3));
        entry.t1 = parseInt(columns.at(4));
        entry.t2 = parseInt(columns.at(5));
        entry.t3 = parseInt(columns.at(6));
        entry.t4 = parseInt(columns.at(7));
        entry.ambient = parseInt(columns.at(8));
        data.append(entry);
    }

    return data;
}

QStringList DataManager::checkWagonIssues(const QString &trainId, const QString &wagonId)
{
    const QList<LogEntry> data = loadLogData(trainId, wagonId);
    if (data.isEmpty()) {
        return {};
    }

    bool hasPressureIssue = false;
    bool hasRodIssue = false;
    bool hasTemperatureIssue = false;

    for (const LogEntry &row : data) {
        if ((row.pipePressure > 4.8 && row.cylinderPressure > 0.4)
            || (row.pipePressure < 4.2 && row.cylinderPressure < 0.5)
            || row.cylinderPressure > 4.5) {
            hasPressureIssue = true;
        }

        if (row.rod > 0 && (row.rod < 75 || row.rod > 125)) {
            hasRodIssue = true;
        }

        if (std::max({row.t1, row.t2, row.t3, row.t4}) >= 80) {
            hasTemperatureIssue = true;
        }

        if (hasPressureIssue && hasRodIssue && hasTemperatureIssue) {
            break;
        }
    }

    QStringList issues;
    if (hasRodIssue) {
        issues.append(formatIssueCode(QStringLiteral("L")));
    }
    if (hasPressureIssue) {
        issues.append(formatIssueCode(QStringLiteral("P")));
    }
    if (hasTemperatureIssue) {
        issues.append(formatIssueCode(QStringLiteral("T")));
    }

    return issues;
}
