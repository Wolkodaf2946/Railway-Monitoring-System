#include "ui/SensorTable.h"

#include <QAbstractItemView>
#include <QHeaderView>

SensorTable::SensorTable(const QStringList &columns, QWidget *parent)
    : QTableWidget(parent)
{
    setColumnCount(columns.size());
    setHorizontalHeaderLabels(columns);
    verticalHeader()->setVisible(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *header = horizontalHeader();
    for (int i = 0; i < columns.size(); ++i) {
        if (i == 0) {
            setColumnWidth(i, 170);
            header->setSectionResizeMode(i, QHeaderView::Interactive);
        } else {
            header->setSectionResizeMode(i, QHeaderView::Stretch);
        }
    }

    setStyleSheet(QStringLiteral(
        "QTableWidget {"
        "  border: 1px solid #ddd;"
        "  background-color: white;"
        "  gridline-color: #eee;"
        "}"
        "QHeaderView::section {"
        "  background-color: #f1f3f5;"
        "  padding: 6px;"
        "  border: 1px solid #ddd;"
        "  font-weight: bold;"
        "  color: #555555;"
        "}"
        "QTableWidget::item {"
        "  padding-left: 5px;"
        "}"));
    setMinimumHeight(200);
}
