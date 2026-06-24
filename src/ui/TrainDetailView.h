#pragma once

#include <QHash>
#include <QWidget>

class QListWidget;
class QStackedWidget;

class WagonView;

class TrainDetailView : public QWidget {
    Q_OBJECT

public:
    explicit TrainDetailView(const QString &trainId, QWidget *parent = nullptr);

private slots:
    void onWagonSelected(int index);

private:
    void setupUi();
    void loadWagonsList();

    QString m_trainId;
    QStackedWidget *m_contentArea = nullptr;
    QListWidget *m_wagonList = nullptr;
    QHash<QString, WagonView *> m_loadedViews;
};
