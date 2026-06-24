#pragma once

#include <QString>

struct LogEntry {
    QString time;
    double pipePressure = 0.0;
    double cylinderPressure = 0.0;
    int rod = 0;
    int t1 = 0;
    int t2 = 0;
    int t3 = 0;
    int t4 = 0;
    int ambient = 0;
};
