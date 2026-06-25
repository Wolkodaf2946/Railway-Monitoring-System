// Логика считывания данных из файлов логов и проверки проблем вагонов
#include "core/DataManager.h"

#include <algorithm>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringConverter>
#include <QTextStream>

namespace { // функции доступны только в этом файле:

// Функция для безопасного преобразования строки в double с возможностью указания значения по умолчанию
double parseDouble(const QString &value, double fallback = 0.0)
{
    bool ok = false;
    const double parsed = value.toDouble(&ok);
    // return parsed if ok else fallback
    return ok ? parsed : fallback;
}

// Функция для безопасного преобразования строки в int с возможностью указания значения по умолчанию
int parseInt(const QString &value, int fallback = 0)
{
    bool ok = false;
    const int parsed = value.toInt(&ok);
    // return parsed if ok else fallback
    return ok ? parsed : fallback;
}

// Функция для форматирования кода проблемы: удаляет пробелы и переводит в верхний регистр
QString formatIssueCode(const QString &code)
{
    // убрали пробелы и перевели в верхний регистр
    return code.trimmed().toUpper();
}

} // namespace

// Метод для получения пути к директории с логами
QString DataManager::logsDirPath()
{
    // получаем путь к текущей директрии
    // склеиваем текущий путь с "data_logs" и возвращаем полный путь
    return QDir::current().filePath(QStringLiteral("data_logs"));
}

// Метод для получения списка поездов
QStringList DataManager::getTrains()
{
    // валидация пути к директории с логами
    QDir logsDir(logsDirPath());
    if (!logsDir.exists()) {
        return {};
    }

    QStringList trains;
    // получаем список всех директорий в logsDir
    const QFileInfoList entries = logsDir.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot, // фильтруем только директории, исключая "." и ".."
        QDir::Name // сортируем по имени
    );

    for (const QFileInfo &entry : entries) {
        const QString name = entry.fileName(); // получаем имя директории
        if (name.startsWith(QStringLiteral("train_"))) { // если имя директории начинается с "train_"
            // QStringLiteral("train_") - создание строки "train_"
            // QStringLiteral("train_").size() - длина строки "train_" (6)
            // name.mid(...) - возвращает подстроку имени директории, начиная с индекса
            trains.append(name.mid(QStringLiteral("train_").size()));
        }
    }
    trains.sort();
    return trains;
}

// Метод для получения списка вагонов для конкретного поезда
QStringList DataManager::getWagons(const QString &trainId)
{
    // создаём путь до вагона и валидируем
    QDir trainDir(QDir(logsDirPath()).filePath(QStringLiteral("train_%1").arg(trainId)));
    if (!trainDir.exists()) {
        return {};
    }

    QStringList wagons;
    const QFileInfoList entries = trainDir.entryInfoList(
        {QStringLiteral("*.log")}, // маска для поиска файлов с расширением .log
        QDir::Files, // фильтруем только файлы
        QDir::Name // сортируем по имени
    );

    // проходим по всем найденным файлам и добавляем их имена (без расширения) в список wagons
    for (const QFileInfo &entry : entries) {
        wagons.append(entry.completeBaseName());
    }

    wagons.sort();
    return wagons;
}

// Метод для загрузки данных логов для конкретного поезда и вагона
QList<LogEntry> DataManager::loadLogData(const QString &trainId, const QString &wagonId)
{
    const QString filePath = QDir(logsDirPath())
                                 .filePath(QStringLiteral("train_%1/%2.log").arg(trainId, wagonId));

    QFile file(filePath);
    if (!file.exists() || !file.open(
        QIODevice::ReadOnly | QIODevice::Text // только ЧИТАЕМ ТЕКСТОВЫЙ файл
    )) {
        return {};
    }

    QTextStream stream(&file); // подключаем поток к файлу
    stream.setEncoding(QStringConverter::Utf8); // указываем кодировку UTF-8 для корректного чтения текста

    QList<LogEntry> data;
    if (stream.atEnd()) {
        return data;
    }

    stream.readLine(); // пропускаем первую строку (заголовок)


    while (!stream.atEnd()) {
        // читаем строку и удаляем лишние пробелы
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

// Метод для проверки проблем вагона по данным логов
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
        if (
            (row.pipePressure > 4.8 && row.cylinderPressure > 0.4) // свмопроизвольное торможение
            || (row.pipePressure < 4.2 && row.cylinderPressure < 0.5) // отказ тормозов
            || row.cylinderPressure > 4.5 // критическое давление (такое почти не бывает, но учитывать стоит)
        ) {
            hasPressureIssue = true;
        }

        if (row.rod > 0 && (row.rod < 75 || row.rod > 125)) { // ход штока тормозного цилиндра вне допустимого диапазона
            hasRodIssue = true;
        }

        if (std::max({row.t1, row.t2, row.t3, row.t4}) >= 80) { // температура на колесных парах выше допустимого значения
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
