#include "ui/CollapsibleBox.h"

#include <QToolButton>
#include <QVBoxLayout>

// title - текст кнопки-заголовка, contentWidget - то, что прячется/показывается внутри
CollapsibleBox::CollapsibleBox(const QString &title, QWidget *contentWidget, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this); // кнопка сверху, контент снизу - вертикально
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // QToolButton используем как кликабельный заголовок секции (а не как обычную кнопку с действием)
    m_toggleButton = new QToolButton(this);
    m_toggleButton->setText(title);
    m_toggleButton->setCheckable(true); // делает кнопку переключаемой (toggle), а не одноразовой
    m_toggleButton->setChecked(true); // по умолчанию секция развёрнута
    m_toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // текст рядом со стрелкой-иконкой
    m_toggleButton->setArrowType(Qt::DownArrow); // стрелка вниз = "развёрнуто"
    m_toggleButton->setStyleSheet(QStringLiteral(
        "QToolButton {"
        "  border: none;"
        "  background-color: #ddd;"
        "  color: #333;"
        "  font-weight: bold;"
        "  padding: 5px;"
        "  text-align: left;"
        "}"
        "QToolButton:checked { background-color: #ccc; }" // визуально отличаем "нажатое" (развёрнутое) состояние
        "QToolButton:hover { background-color: #eee; }"));

    // toggled - сигнал при смене состояния (развёрнуто/свёрнуто), передаёт новое bool-значение
    connect(m_toggleButton, &QToolButton::toggled, this, &CollapsibleBox::onToggle);
    layout->addWidget(m_toggleButton);

    // m_contentArea - отдельный контейнер-обёртка, который мы целиком показываем/скрываем
    m_contentArea = new QWidget(this);
    m_contentLayout = new QVBoxLayout(m_contentArea);
    m_contentLayout->setContentsMargins(0, 0, 0, 10); // небольшой отступ снизу под содержимым
    if (contentWidget != nullptr) {
        m_contentLayout->addWidget(contentWidget); // вкладываем переданный снаружи виджет (например SensorTable)
    }

    layout->addWidget(m_contentArea);
}

// слот: checked = true когда секция развёрнута (кнопка "нажата"), false когда свёрнута
void CollapsibleBox::onToggle(bool checked)
{
    m_toggleButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow); // тернарный оператор: ? : - как if/else в одну строку
    m_contentArea->setVisible(checked); // скрыть/показать весь контент одним вызовом
}
