#include "ui/MainWindow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

#include "core/DataManager.h"
#include "ui/TrainDetailView.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Railway Monitoring System"));
    resize(1280, 800); // размер окна
    setupUi(); // отрисовка UI
    loadTrains(); // загрузка списка поездов
}

void MainWindow::setupUi()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget); // единственный центральный виджет QMainWindow

    // горизонтальный layout для центрального виджета
    auto *mainLayout = new QHBoxLayout(centralWidget);
    // убираем отступы и пробелы между виджетами
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // splitter - две панели с двигаемой границей
    auto *splitter = new QSplitter(Qt::Horizontal, centralWidget);

    // левая панель (фон + список составов)
    auto *leftPanel = new QWidget(splitter);
    leftPanel->setStyleSheet(QStringLiteral("background-color: #f8f9fa; border-right: 1px solid #c0c0c0;"));
    auto *leftLayout = new QVBoxLayout(leftPanel); // вертикальная раскладка внутри левой панели
    leftLayout->setContentsMargins(0, 0, 0, 0);

    auto *listLabel = new QLabel(QStringLiteral("СПИСОК СОСТАВОВ"), leftPanel);
    listLabel->setStyleSheet(QStringLiteral(
        "padding: 15px 10px 5px 10px;"
        "color: #666;"
        "font-weight: bold;"
        "font-size: 11px;"));
    leftLayout->addWidget(listLabel);

    // список составов
    m_trainList = new QListWidget(leftPanel);
    m_trainList->setFocusPolicy(Qt::NoFocus); // убираем пунктирную рамку фокуса
    m_trainList->setStyleSheet(QStringLiteral( // стили списка и его item'ов (::item, :hover, :selected)
        "QListWidget {"
        "  border: none;"
        "  background: transparent;"
        "}"
        "QListWidget::item {"
        "  padding: 15px 10px;"
        "  border-bottom: 1px solid #e0e0e0;"
        "  color: #333;"
        "  font-size: 14px;"
        "}"
        "QListWidget::item:hover { background-color: #e9ecef; }"
        "QListWidget::item:selected {"
        "  background-color: #0078d7;"
        "  color: white;"
        "  border-left: 5px solid #004578;"
        "}"));


    // сигнал->слот: смена выбранной строки вызывает changeTrainView
    // connect(отправитель, &Сигнал, получатель, &Слот);
    connect(m_trainList, &QListWidget::currentRowChanged, this, &MainWindow::changeTrainView);
    leftLayout->addWidget(m_trainList);

    splitter->addWidget(leftPanel); // первая створка splitter'а (левая)

    // стек экранов составов (показывает один виджет из многих по индексу)
    m_stack = new QStackedWidget(splitter);
    m_stack->addWidget(new QLabel(QStringLiteral("Выберите состав"), m_stack)); // заглушка, индекс 0
    splitter->addWidget(m_stack); // вторая створка splitter'а (правая)
    splitter->setStretchFactor(1, 4); // правая область получает больше места при ресайзе

    mainLayout->addWidget(splitter);
}

void MainWindow::loadTrains()
{
    const QStringList trains = DataManager::getTrains();
    if (trains.isEmpty()) {
        m_trainList->addItem(QStringLiteral("Нет данных (.log)"));
        return;
    }

    // для каждого поезда: текстовый item слева + реальный экран в стеке справа (создаются сразу, не лениво)
    for (const QString &trainId : trains) {
        m_trainList->addItem(QStringLiteral("Состав № %1").arg(trainId));
        m_stack->addWidget(new TrainDetailView(trainId, m_stack));
    }
}

// слот: index - новая выбранная строка списка (-1 если выбор сброшен)
void MainWindow::changeTrainView(int index)
{
    if (index >= 0) {
        m_stack->setCurrentIndex(index + 1); // +1 из-за заглушки на индексе 0
    }
}
