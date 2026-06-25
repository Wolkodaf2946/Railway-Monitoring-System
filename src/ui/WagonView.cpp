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

// список инициализации с тремя полями: база + два своих поля (порядок как в .h)
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

    // верхняя панель фильтров - QHBoxLayout без родителя, добавится в mainLayout как layout, не как виджет
    auto *topPanel = new QHBoxLayout();
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(QStringLiteral("Поиск по времени..."));

    // textChanged - сигнал срабатывает при каждом изменении текста (на каждое нажатие клавиши)
    // connect(отправитель, &Сигнал, получатель, &Слот);
    connect(m_searchInput, &QLineEdit::textChanged, this, &WagonView::filterLogs);

    m_errorsOnlyCheck = new QCheckBox(QStringLiteral("Только показания с неисправностями"), this);
    connect(m_errorsOnlyCheck, &QCheckBox::checkStateChanged, this, &WagonView::filterLogs);

    topPanel->addWidget(new QLabel(QStringLiteral("Фильтр времени:"), this));
    topPanel->addWidget(m_searchInput);
    topPanel->addSpacing(20);
    topPanel->addWidget(m_errorsOnlyCheck);
    mainLayout->addLayout(topPanel); // вкладываем layout в layout

    // QScrollArea - область с прокруткой для контента, который может не влезать по высоте
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true); // внутренний виджет растягивается по ширине scroll-области
    scroll->setFrameShape(QFrame::NoFrame); // без рамки вокруг scroll-области

    auto *contentWidget = new QWidget(scroll);
    auto *contentLayout = new QVBoxLayout(contentWidget);

    // создаём три таблицы датчиков с разными наборами колонок (см. Constants.h)
    m_brakesTable = new SensorTable(kBrakeColumns, contentWidget);
    m_rodTable = new SensorTable(kRodColumns, contentWidget);
    m_tempTable = new SensorTable(kTempColumns, contentWidget);
    m_tables = {m_brakesTable, m_rodTable, m_tempTable}; // список для общих операций (фильтрация)

    // каждая таблица оборачивается в сворачиваемую секцию (аккордеон)
    contentLayout->addWidget(new CollapsibleBox(QStringLiteral("Тормозная система (ВР №483)"), m_brakesTable, contentWidget));
    contentLayout->addWidget(new CollapsibleBox(QStringLiteral("Состояние штока"), m_rodTable, contentWidget));
    contentLayout->addWidget(new CollapsibleBox(QStringLiteral("Температура букс/колес"), m_tempTable, contentWidget));
    contentLayout->addStretch(); // "пружина" - забирает всё лишнее пустое место внизу

    scroll->setWidget(contentWidget); // contentWidget - то, что реально скроллится
    mainLayout->addWidget(scroll);
}

void WagonView::loadData()
{
    const QList<LogEntry> logs = DataManager::loadLogData(m_trainId, m_wagonNumber);
    const int rowsCount = logs.size(); // кол-во строк
    
    // задаём кол-во строк в таблицах:
    m_brakesTable->setRowCount(rowsCount); 
    m_rodTable->setRowCount(rowsCount);
    m_tempTable->setRowCount(rowsCount);

    for (int i = 0; i < rowsCount; ++i) {
        const LogEntry &row = logs.at(i); // ссылка на запись, без копирования

        // --- блок тормозов ---
        auto *brakeTimeItem = new QTableWidgetItem(row.time);
        auto *cylinderItem = new QTableWidgetItem(QString::number(row.cylinderPressure)); // double -> QString
        auto *pipeItem = new QTableWidgetItem(QString::number(row.pipePressure));

        m_brakesTable->setItem(i, 0, brakeTimeItem); // setItem(строка, колонка, item) - вставка в ячейку таблицы
        m_brakesTable->setItem(i, 1, cylinderItem);
        m_brakesTable->setItem(i, 2, pipeItem);

        QString brakeStatus = QStringLiteral("Норма");
        bool brakeError = false;

        // проверка трёх видов неисправности тормозов по показаниям давления
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
            setAlert(brakeStatusItem); // подсветка статус-ячейки
            setAlert(cylinderItem); // подсветка ячейки с "виновным" значением
            markRowAsError(m_brakesTable, i); // помечаем строку для фильтра "только ошибки"
        }
        m_brakesTable->setItem(i, 3, brakeStatusItem);

        // --- блок штока ---
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

        // --- блок температуры (4 колеса в одной строке) ---
        auto *tempTimeItem = new QTableWidgetItem(row.time);
        m_tempTable->setItem(i, 0, tempTimeItem);

        const QList<int> temps = {row.t1, row.t2, row.t3, row.t4}; // список инициализации (как []int{...} в Go)
        bool tempIssue = false;
        for (int column = 0; column < temps.size(); ++column) {
            auto *tempItem = new QTableWidgetItem(QString::number(temps.at(column)));
            if (temps.at(column) >= 80) {
                setAlert(tempItem);
                tempIssue = true;
            }
            m_tempTable->setItem(i, column + 1, tempItem); // +1 - колонка 0 занята временем
        }
        if (tempIssue) {
            markRowAsError(m_tempTable, i);
        }
    }
}

// подсветка одной ячейки красным (визуальный алерт)
void WagonView::setAlert(QTableWidgetItem *item)
{
    if (item == nullptr) {
        return;
    }
    item->setBackground(QColor(Qt::red));
    item->setForeground(QColor(Qt::white));
    item->setToolTip(QStringLiteral("Внимание! Выход за пределы нормы.")); // подсказка при наведении мышью
}

// помечаем item времени в колонке 0 как "строка с ошибкой" - используется потом в filterLogs
void WagonView::markRowAsError(SensorTable *table, int rowIndex)
{
    if (table == nullptr) {
        return;
    }

    QTableWidgetItem *timeItem = table->item(rowIndex, 0);
    if (timeItem != nullptr) {
        timeItem->setData(Qt::UserRole, true); // прячем флаг "ошибка" в UserRole, как wagonId в TrainDetailView
    }
}

// слот: вызывается и при наборе текста в поиске, и при переключении чекбокса "только ошибки"
void WagonView::filterLogs()
{
    const QString searchText = m_searchInput->text().trimmed().toLower(); // нормализуем для поиска без учёта регистра
    const bool showErrorsOnly = m_errorsOnlyCheck->isChecked();

    for (SensorTable *table : std::as_const(m_tables)) { // проходим по всем трём таблицам сразу
        if (table == nullptr) {
            continue;
        }

        for (int row = 0; row < table->rowCount(); ++row) {
            QTableWidgetItem *timeItem = table->item(row, 0); // время - всегда в колонке 0
            if (timeItem == nullptr) {
                continue;
            }

            bool textMatch = true;
            if (!searchText.isEmpty()) {
                textMatch = timeItem->text().toLower().contains(searchText);
            }

            bool errorMatch = true;
            if (showErrorsOnly) {
                errorMatch = timeItem->data(Qt::UserRole).toBool(); // достаём флаг, поставленный в markRowAsError
            }

            table->setRowHidden(row, !(textMatch && errorMatch)); // скрываем строку, если хоть одно условие не подошло
        }
    }
}
