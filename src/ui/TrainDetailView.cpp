#include "ui/TrainDetailView.h"

#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "core/DataManager.h"
#include "ui/WagonView.h"

TrainDetailView::TrainDetailView(const QString &trainId, QWidget *parent)
    : QWidget(parent)
    , m_trainId(trainId)
{
    setupUi();
    loadWagonsList();
}

void TrainDetailView::setupUi()
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    auto *splitter = new QSplitter(Qt::Horizontal, this);

    m_contentArea = new QStackedWidget(splitter);
    auto *placeholder = new QLabel(QStringLiteral("Состав %1\nВыберите вагон из списка справа ->").arg(m_trainId), splitter);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet(QStringLiteral("font-size: 16px; color: #777;"));
    m_contentArea->addWidget(placeholder);
    splitter->addWidget(m_contentArea);

    auto *rightPanel = new QWidget(splitter);
    rightPanel->setStyleSheet(QStringLiteral("background-color: #f0f0f0; border-left: 1px solid #ccc;"));
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto *header = new QLabel(QStringLiteral("ВАГОНЫ"), rightPanel);
    header->setStyleSheet(QStringLiteral("font-weight: bold; padding: 10px; color: #555;"));
    rightLayout->addWidget(header);

    m_wagonList = new QListWidget(rightPanel);
    m_wagonList->setFixedWidth(220);
    m_wagonList->setStyleSheet(QStringLiteral(
        "QListWidget { border: none; background: transparent; }"
        "QListWidget::item {"
        "  padding: 8px;"
        "  border-bottom: 1px solid #ddd;"
        "  margin: 2px 5px;"
        "  border-radius: 4px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #0078d7;"
        "  color: white;"
        "}"
        "QListWidget::item:hover { background-color: #e0e0e0; }"));
    connect(m_wagonList, &QListWidget::currentRowChanged, this, &TrainDetailView::onWagonSelected);
    rightLayout->addWidget(m_wagonList);

    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 0);

    layout->addWidget(splitter);
}

void TrainDetailView::loadWagonsList()
{
    m_wagonList->clear();
    const QStringList wagons = DataManager::getWagons(m_trainId);

    if (wagons.isEmpty()) {
        m_wagonList->addItem(QStringLiteral("Нет данных"));
        return;
    }

    for (const QString &wagonId : wagons) {
        auto *item = new QListWidgetItem();
        item->setData(Qt::UserRole, wagonId);

        const QStringList issues = DataManager::checkWagonIssues(m_trainId, wagonId);
        if (!issues.isEmpty()) {
            item->setText(QStringLiteral("Вагон № %1 [%2]").arg(wagonId, issues.join(QStringLiteral(", "))));
            item->setBackground(QColor(QStringLiteral("#ffcccc")));
            item->setForeground(QColor(QStringLiteral("#cc0000")));
        } else {
            item->setText(QStringLiteral("Вагон № %1").arg(wagonId));
            item->setBackground(QColor(QStringLiteral("#e8f5e9")));
        }

        m_wagonList->addItem(item);
    }
}

void TrainDetailView::onWagonSelected(int index)
{
    if (index < 0) {
        return;
    }

    QListWidgetItem *item = m_wagonList->item(index);
    if (item == nullptr) {
        return;
    }

    const QString wagonId = item->data(Qt::UserRole).toString();
    if (wagonId.isEmpty()) {
        return;
    }

    if (m_loadedViews.contains(wagonId)) {
        m_contentArea->setCurrentWidget(m_loadedViews.value(wagonId));
        return;
    }

    auto *view = new WagonView(m_trainId, wagonId, m_contentArea);
    m_loadedViews.insert(wagonId, view);
    m_contentArea->addWidget(view);
    m_contentArea->setCurrentWidget(view);
}
