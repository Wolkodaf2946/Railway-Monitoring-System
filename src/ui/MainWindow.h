#pragma once

#include <QMainWindow>

class QListWidget;
class QStackedWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void changeTrainView(int index);

private:
    void setupUi();
    void loadTrains();

    QListWidget *m_trainList = nullptr;
    QStackedWidget *m_stack = nullptr;
};
