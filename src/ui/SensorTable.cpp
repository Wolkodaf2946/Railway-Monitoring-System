#include "ui/SensorTable.h"

#include <QAbstractItemView>
#include <QHeaderView>

// columns - заголовки колонок, передаются снаружи (kBrakeColumns/kRodColumns/kTempColumns)
SensorTable::SensorTable(const QStringList &columns, QWidget *parent)
    : QTableWidget(parent) // наследуемся от QTableWidget - вся базовая логика таблицы уже готова
{
    setColumnCount(columns.size());
    setHorizontalHeaderLabels(columns); // подписи колонок сразу из переданного списка
    verticalHeader()->setVisible(false); // прячем нумерацию строк слева (1,2,3...)
    setSelectionBehavior(QAbstractItemView::SelectRows); // клик выделяет всю строку, а не одну ячейку
    setEditTriggers(QAbstractItemView::NoEditTriggers); // таблица только для просмотра, редактировать нельзя

    auto *header = horizontalHeader(); // указатель на заголовок таблицы (строка с названиями колонок)
    for (int i = 0; i < columns.size(); ++i) {
        if (i == 0) {
            setColumnWidth(i, 170); // первая колонка (время) - фиксированная стартовая ширина
            header->setSectionResizeMode(i, QHeaderView::Interactive); // но юзер может потянуть и поменять
        } else {
            header->setSectionResizeMode(i, QHeaderView::Stretch); // остальные тянутся, заполняя свободное место
        }
    }

    setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "  border: 1px solid #ddd;"
        "  background-color: white;"
        "  gridline-color: #eee;"
        "}"
        "QHeaderView::section {" // стиль заголовков колонок
        "  background-color: #f1f3f5;"
        "  padding: 6px;"
        "  border: 1px solid #ddd;"
        "  font-weight: bold;"
        "  color: #555555;"
        "}"
        "QTableWidget::item {"
        "  padding-left: 5px;"
        "}"));
    setMinimumHeight(200); // не даём таблице сжаться меньше этой высоты
}
