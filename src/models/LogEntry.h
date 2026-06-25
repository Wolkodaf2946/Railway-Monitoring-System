#pragma once

#include <QString>

struct LogEntry {
    QString time; // время записи лога
    double pipePressure = 0.0; // давление в тормозной магистрали
    double cylinderPressure = 0.0; // давление в тормозном цилиндре
    int rod = 0; // ход штока тормозного цилиндра
    // температура на колесных парах (t1, t2, t3, t4) и температура окружающей среды (ambient)
    int t1 = 0; 
    int t2 = 0;
    int t3 = 0;
    int t4 = 0;
    int ambient = 0;
};
