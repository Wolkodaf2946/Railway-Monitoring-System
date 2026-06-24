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
    resize(1280, 800);
    setupUi();
    loadTrains();
}

void MainWindow::setupUi()
{
    auto *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    auto *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *splitter = new QSplitter(Qt::Horizontal, centralWidget);

    auto *leftPanel = new QWidget(splitter);
    leftPanel->setStyleSheet(QStringLiteral("background-color: #f8f9fa; border-right: 1px solid #c0c0c0;"));
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    auto *listLabel = new QLabel(QStringLiteral("СПИСОК СОСТАВОВ"), leftPanel);
    listLabel->setStyleSheet(QStringLiteral(
        "padding: 15px 10px 5px 10px;"
        "color: #666;"
        "font-weight: bold;"
        "font-size: 11px;"));
    leftLayout->addWidget(listLabel);

    m_trainList = new QListWidget(leftPanel);
    m_trainList->setFocusPolicy(Qt::NoFocus);
    m_trainList->setStyleSheet(QStringLiteral(
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
    connect(m_trainList, &QListWidget::currentRowChanged, this, &MainWindow::changeTrainView);
    leftLayout->addWidget(m_trainList);

    splitter->addWidget(leftPanel);

    m_stack = new QStackedWidget(splitter);
    m_stack->addWidget(new QLabel(QStringLiteral("Выберите состав"), m_stack));
    splitter->addWidget(m_stack);
    splitter->setStretchFactor(1, 4);

    mainLayout->addWidget(splitter);
}

void MainWindow::loadTrains()
{
    const QStringList trains = DataManager::getTrains();
    if (trains.isEmpty()) {
        m_trainList->addItem(QStringLiteral("Нет данных (.log)"));
        return;
    }

    for (const QString &trainId : trains) {
        m_trainList->addItem(QStringLiteral("Состав № %1").arg(trainId));
        m_stack->addWidget(new TrainDetailView(trainId, m_stack));
    }
}

void MainWindow::changeTrainView(int index)
{
    if (index >= 0) {
        m_stack->setCurrentIndex(index + 1);
    }
}
