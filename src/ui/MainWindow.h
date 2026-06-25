#pragma once

#include <QMainWindow>

class QListWidget;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT // макрос, необходимый для работы сигналов и слотов

public:
    // конструктор с предотвращением неявной конвертации
    explicit MainWindow(QWidget *parent = nullptr);

// приватные методы для слотов
private slots:
    void changeTrainView(int index);

private:
    void setupUi();
    void loadTrains();

    // "бронирование" памяти для списка поездов и стека виджетов
    QListWidget *m_trainList = nullptr;
    QStackedWidget *m_stack = nullptr;
};
