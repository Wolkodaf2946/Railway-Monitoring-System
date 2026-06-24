#include "ui/WagonView.h"

#include <QCheckBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QScrollArea>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include "core/DataManager.h"
#include "models/Constants.h"
#include "ui/CollapsibleBox.h"
#include "ui/SensorTable.h"

WagonView::WagonView(const QString &trainId, const QString &wagonNumber, QWidget *parent)
    : QWidget(parent)
    , m_trainId(trainId)
    , m_wagonNumber(wagonNumber)
{
    setupUi();
    loadData();
}

void WagonView::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *topPanel = new QHBoxLayout();
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(QStringLiteral("Поиск по времени..."));
    connect(m_searchInput, &QLineEdit::textChanged, this, &WagonView::filterLogs);

    m_errorsOnlyCheck = new QCheckBox(QStringLiteral("Только показания с неисправностями"), this);
    connect(m_errorsOnlyCheck, &QCheckBox::checkStateChanged, this, &WagonView::filterLogs);

    topPanel->addWidget(new QLabel(QStringLiteral("Фильтр времени:"), this));
    topPanel->addWidget(m_searchInput);
    topPanel->addSpacing(20);
    topPanel->addWidget(m_errorsOnlyCheck);
    mainLayout->addLayout(topPanel);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    auto *contentWidget = new QWidget(scroll);
    auto *contentLayout = new QVBoxLayout(contentWidget);

    m_brakesTable = new SensorTable(kBrakeColumns, contentWidget);
    m_rodTable = new SensorTable(kRodColumns, contentWidget);
    m_tempTable = new SensorTable(kTempColumns, contentWidget);
    m_tables = {m_brakesTable, m_rodTable, m_tempTable};

    contentLayout->addWidget(new CollapsibleBox(QStringLiteral("Тормозная система (ВР №483)"), m_brakesTable, contentWidget));
    contentLayout->addWidget(new CollapsibleBox(QStringLiteral("Состояние штока"), m_rodTable, contentWidget));
    contentLayout->addWidget(new CollapsibleBox(QStringLiteral("Температура букс/колес"), m_tempTable, contentWidget));
    contentLayout->addStretch();

    scroll->setWidget(contentWidget);
    mainLayout->addWidget(scroll);
}

void WagonView::loadData()
{
    const QList<LogEntry> logs = DataManager::loadLogData(m_trainId, m_wagonNumber);
    const int rowsCount = logs.size();

    m_brakesTable->setRowCount(rowsCount);
    m_rodTable->setRowCount(rowsCount);
    m_tempTable->setRowCount(rowsCount);

    for (int i = 0; i < rowsCount; ++i) {
        const LogEntry &row = logs.at(i);

        auto *brakeTimeItem = new QTableWidgetItem(row.time);
        auto *cylinderItem = new QTableWidgetItem(QString::number(row.cylinderPressure));
        auto *pipeItem = new QTableWidgetItem(QString::number(row.pipePressure));

        m_brakesTable->setItem(i, 0, brakeTimeItem);
        m_brakesTable->setItem(i, 1, cylinderItem);
        m_brakesTable->setItem(i, 2, pipeItem);

        QString brakeStatus = QStringLiteral("Норма");
        bool brakeError = false;

        if (row.cylinderPressure > 4.5) {
            brakeStatus = QStringLiteral("ПЕРЕДАВЛЕНИЕ (РЕЖИМ)");
            brakeError = true;
        } else if (row.pipePressure > 4.8 && row.cylinderPressure > 0.4) {
            brakeStatus = QStringLiteral("НЕОТПУСК / САМОСРАБ");
            brakeError = true;
        } else if (row.pipePressure < 4.2 && row.cylinderPressure < 0.5) {
            brakeStatus = QStringLiteral("ОТКАЗ ТОРМОЖЕНИЯ");
            brakeError = true;
        }

        auto *brakeStatusItem = new QTableWidgetItem(brakeStatus);
        if (brakeError) {
            setAlert(brakeStatusItem);
            setAlert(cylinderItem);
            markRowAsError(m_brakesTable, i);
        }
        m_brakesTable->setItem(i, 3, brakeStatusItem);

        auto *rodTimeItem = new QTableWidgetItem(row.time);
        auto *rodItem = new QTableWidgetItem(QString::number(row.rod));
        QString rodStatus = QStringLiteral("Поездное положение");
        bool rodError = false;

        if (row.rod > 0) {
            if (row.rod >= 75 && row.rod <= 125) {
                rodStatus = QStringLiteral("Торможение (Норма)");
            } else {
                rodStatus = QStringLiteral("НЕНОРМ. ВЫХОД");
                rodError = true;
            }
        }

        auto *rodStatusItem = new QTableWidgetItem(rodStatus);
        m_rodTable->setItem(i, 0, rodTimeItem);
        m_rodTable->setItem(i, 1, rodItem);
        if (rodError) {
            setAlert(rodItem);
            setAlert(rodStatusItem);
            markRowAsError(m_rodTable, i);
        }
        m_rodTable->setItem(i, 2, rodStatusItem);

        auto *tempTimeItem = new QTableWidgetItem(row.time);
        m_tempTable->setItem(i, 0, tempTimeItem);

        const QList<int> temps = {row.t1, row.t2, row.t3, row.t4};
        bool tempIssue = false;
        for (int column = 0; column < temps.size(); ++column) {
            auto *tempItem = new QTableWidgetItem(QString::number(temps.at(column)));
            if (temps.at(column) >= 80) {
                setAlert(tempItem);
                tempIssue = true;
            }
            m_tempTable->setItem(i, column + 1, tempItem);
        }
        if (tempIssue) {
            markRowAsError(m_tempTable, i);
        }
    }
}

void WagonView::setAlert(QTableWidgetItem *item)
{
    if (item == nullptr) {
        return;
    }

    item->setBackground(QColor(Qt::red));
    item->setForeground(QColor(Qt::white));
    item->setToolTip(QStringLiteral("Внимание! Выход за пределы нормы."));
}

void WagonView::markRowAsError(SensorTable *table, int rowIndex)
{
    if (table == nullptr) {
        return;
    }

    QTableWidgetItem *timeItem = table->item(rowIndex, 0);
    if (timeItem != nullptr) {
        timeItem->setData(Qt::UserRole, true);
    }
}

void WagonView::filterLogs()
{
    const QString searchText = m_searchInput->text().trimmed().toLower();
    const bool showErrorsOnly = m_errorsOnlyCheck->isChecked();

    for (SensorTable *table : std::as_const(m_tables)) {
        if (table == nullptr) {
            continue;
        }

        for (int row = 0; row < table->rowCount(); ++row) {
            QTableWidgetItem *timeItem = table->item(row, 0);
            if (timeItem == nullptr) {
                continue;
            }

            bool textMatch = true;
            if (!searchText.isEmpty()) {
                textMatch = timeItem->text().toLower().contains(searchText);
            }

            bool errorMatch = true;
            if (showErrorsOnly) {
                errorMatch = timeItem->data(Qt::UserRole).toBool();
            }

            table->setRowHidden(row, !(textMatch && errorMatch));
        }
    }
}
