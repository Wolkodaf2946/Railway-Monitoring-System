#include "ui/CollapsibleBox.h"

#include <QToolButton>
#include <QVBoxLayout>

CollapsibleBox::CollapsibleBox(const QString &title, QWidget *contentWidget, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_toggleButton = new QToolButton(this);
    m_toggleButton->setText(title);
    m_toggleButton->setCheckable(true);
    m_toggleButton->setChecked(true);
    m_toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_toggleButton->setArrowType(Qt::DownArrow);
    m_toggleButton->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  border: none;"
        "  background-color: #ddd;"
        "  color: #333;"
        "  font-weight: bold;"
        "  padding: 5px;"
        "  text-align: left;"
        "}"
        "QToolButton:checked { background-color: #ccc; }"
        "QToolButton:hover { background-color: #eee; }"));

    connect(m_toggleButton, &QToolButton::toggled, this, &CollapsibleBox::onToggle);
    layout->addWidget(m_toggleButton);

    m_contentArea = new QWidget(this);
    m_contentLayout = new QVBoxLayout(m_contentArea);
    m_contentLayout->setContentsMargins(0, 0, 0, 10);
    if (contentWidget != nullptr) {
        m_contentLayout->addWidget(contentWidget);
    }

    layout->addWidget(m_contentArea);
}

void CollapsibleBox::onToggle(bool checked)
{
    m_toggleButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
    m_contentArea->setVisible(checked);
}
