#pragma once

#include <QWidget>

class QToolButton;
class QVBoxLayout;

class CollapsibleBox : public QWidget {
    Q_OBJECT

public:
    explicit CollapsibleBox(const QString &title, QWidget *contentWidget = nullptr, QWidget *parent = nullptr);

private slots:
    void onToggle(bool checked);

private:
    QToolButton *m_toggleButton = nullptr;
    QWidget *m_contentArea = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
};
