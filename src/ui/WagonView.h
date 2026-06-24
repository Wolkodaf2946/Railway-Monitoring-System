#pragma once

#include <QList>
#include <QWidget>

#include "models/LogEntry.h"

class QCheckBox;
class QLineEdit;
class QTableWidgetItem;

class SensorTable;

class WagonView : public QWidget {
    Q_OBJECT

public:
    explicit WagonView(const QString &trainId, const QString &wagonNumber, QWidget *parent = nullptr);

private slots:
    void filterLogs();

private:
    void setupUi();
    void loadData();
    void setAlert(QTableWidgetItem *item);
    void markRowAsError(SensorTable *table, int rowIndex);

    QString m_trainId;
    QString m_wagonNumber;
    QLineEdit *m_searchInput = nullptr;
    QCheckBox *m_errorsOnlyCheck = nullptr;
    SensorTable *m_brakesTable = nullptr;
    SensorTable *m_rodTable = nullptr;
    SensorTable *m_tempTable = nullptr;
    QList<SensorTable *> m_tables;
};
