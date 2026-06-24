#pragma once

#include <QTableWidget>

class SensorTable : public QTableWidget {
    Q_OBJECT

public:
    explicit SensorTable(const QStringList &columns, QWidget *parent = nullptr);
};
