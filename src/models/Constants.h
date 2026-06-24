#pragma once

#include <QStringList>

inline const QStringList kBrakeColumns = {
    QStringLiteral("Время"),
    QStringLiteral("P торм. цил (атм)"),
    QStringLiteral("P магистрали (атм)"),
    QStringLiteral("Статус")
};

inline const QStringList kRodColumns = {
    QStringLiteral("Время"),
    QStringLiteral("L Выход штока (мм)"),
    QStringLiteral("Статус")
};

inline const QStringList kTempColumns = {
    QStringLiteral("Время"),
    QStringLiteral("T кол.1"),
    QStringLiteral("T кол.2"),
    QStringLiteral("T кол.3"),
    QStringLiteral("T кол.4")
};
