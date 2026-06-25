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

// список инициализации с двумя полями (через запятую): сначала база QWidget, потом m_trainId
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

    // стек для контента вагона (зеркало m_stack из MainWindow, но уровнем ниже)
    m_contentArea = new QStackedWidget(splitter);
    // заглушка, индекс 0 - показывается, пока вагон не выбран
    auto *placeholder = new QLabel(QStringLiteral("Состав %1\nВыберите вагон из списка справа ->").arg(m_trainId), splitter);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet(QStringLiteral("font-size: 16px; color: #777;"));
    m_contentArea->addWidget(placeholder);
    splitter->addWidget(m_contentArea); // левая (основная) створка

    // правая панель со списком вагонов
    auto *rightPanel = new QWidget(splitter);
    rightPanel->setStyleSheet(QStringLiteral("background-color: #f0f0f0; border-left: 1px solid #ccc;"));
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto *header = new QLabel(QStringLiteral("ВАГОНЫ"), rightPanel);
    header->setStyleSheet(QStringLiteral("font-weight: bold; padding: 10px; color: #555;"));
    rightLayout->addWidget(header);

    m_wagonList = new QListWidget(rightPanel);
    m_wagonList->setFixedWidth(220); // фиксированная ширина (в отличие от auto-resize у других виджетов)
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
    // сигнал->слот: смена строки вагона вызывает onWagonSelected
    // connect(отправитель, &Сигнал, получатель, &Слот);
    connect(m_wagonList, &QListWidget::currentRowChanged, this, &TrainDetailView::onWagonSelected);
    rightLayout->addWidget(m_wagonList);

    splitter->addWidget(rightPanel); // правая (узкая) створка
    splitter->setStretchFactor(0, 1); // основная область растягивается
    splitter->setStretchFactor(1, 0); // список вагонов держит фиксированную ширину

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
        auto *item = new QListWidgetItem(); // создаём item отдельно (не через addItem(text))
        // setData(Qt::UserRole, ...) - прячем "сырой" wagonId в item, отдельно от отображаемого текста
        item->setData(Qt::UserRole, wagonId);

        const QStringList issues = DataManager::checkWagonIssues(m_trainId, wagonId);
        if (!issues.isEmpty()) {
            item->setText(QStringLiteral("Вагон № %1 [%2]").arg(wagonId, issues.join(QStringLiteral(", "))));
            item->setBackground(QColor(QStringLiteral("#ffcccc"))); // подсветка вагона с проблемой
            item->setForeground(QColor(QStringLiteral("#cc0000")));
        } else {
            item->setText(QStringLiteral("Вагон № %1").arg(wagonId));
            item->setBackground(QColor(QStringLiteral("#e8f5e9"))); // зелёная подсветка - всё ок
        }

        m_wagonList->addItem(item); // item уже сконфигурирован, просто кладём в список
    }
}

// слот, index - выбранная строка списка вагонов (-1 если сброшен выбор)
void TrainDetailView::onWagonSelected(int index)
{
    if (index < 0) {
        return;
    }

    QListWidgetItem *item = m_wagonList->item(index); // получаем item по индексу
    if (item == nullptr) {
        return;
    }

    // достаём wagonId, который мы спрятали через setData в loadWagonsList
    const QString wagonId = item->data(Qt::UserRole).toString();
    if (wagonId.isEmpty()) {
        return;
    }

    // если экран вагона уже создавался ранее - просто переключаемся на него (без пересчёта данных)
    if (m_loadedViews.contains(wagonId)) {
        m_contentArea->setCurrentWidget(m_loadedViews.value(wagonId));
        return;
    }

    // ленивая загрузка: создаём WagonView только при первом клике на этот вагон
    auto *view = new WagonView(m_trainId, wagonId, m_contentArea);
    m_loadedViews.insert(wagonId, view); // кэшируем в QHash
    m_contentArea->addWidget(view);
    m_contentArea->setCurrentWidget(view);
}
