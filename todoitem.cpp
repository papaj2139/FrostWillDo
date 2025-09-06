#include "todoitem.h"
#include <QApplication>
#include <QDrag>
#include <QMimeData>

TodoItem::TodoItem(const QString &text, QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(40);
    setStyleSheet(
        "TodoItem { "
        "   background-color: #2b2b2b; "
        "   border: 1px solid #404040; "
        "   border-radius: 4px; "
        "   margin: 2px; "
        "   color: #ffffff; "
        "} "
        "TodoItem:hover { "
        "   background-color: #3d3d3d; "
        "}"
    );

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(8);

    m_checkBox = new QCheckBox(this);
    m_checkBox->setFixedSize(20, 20);

    m_label = new QLabel(text, this);
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    //delete button
    m_deleteButton = new QPushButton("×", this);
    m_deleteButton->setFixedSize(20, 20);
    m_deleteButton->setStyleSheet(
        "QPushButton { "
        "   color: #ff6b6b; "
        "   font-weight: bold; "
        "   border: none; "
        "   background-color: transparent; "
        "   border-radius: 10px; "
        "} "
        "QPushButton:hover { "
        "   background-color: #3d1a1a; "
        "}"
    );
    m_deleteButton->hide(); //hidden by default

    layout->addWidget(m_checkBox);
    layout->addWidget(m_label, 1);
    layout->addWidget(m_deleteButton);

    connect(m_checkBox, &QCheckBox::toggled, this, [this](bool checked) {
        if (checked) {
            m_label->setStyleSheet("color: #888; text-decoration: line-through;");
        } else {
            m_label->setStyleSheet("color: #ffffff; text-decoration: none;");
        }
    });

    connect(m_deleteButton, &QPushButton::clicked, this, &TodoItem::deleteRequested);

    updateHeight();
}

void TodoItem::updateHeight()
{
    //calculate required height based on text
    QFontMetrics fm(m_label->font());
    int textWidth = width() - 60; //account for checkbox button margins
    if (textWidth > 0) {
        QRect boundingRect = fm.boundingRect(QRect(0, 0, textWidth, 0),
                                           Qt::AlignLeft | Qt::TextWordWrap,
                                           m_label->text());
        int requiredHeight = qMax(40, boundingRect.height() + 16); //16 for margins
        setFixedHeight(requiredHeight);
    }
}

void TodoItem::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateHeight();
}

void TodoItem::enterEvent(QEnterEvent *event)
{
    m_deleteButton->show();
    QWidget::enterEvent(event);
}

void TodoItem::leaveEvent(QEvent *event)
{
    m_deleteButton->hide();
    QWidget::leaveEvent(event);
}

QString TodoItem::text() const
{
    return m_label->text();
}

void TodoItem::setText(const QString &text)
{
    m_label->setText(text);
    updateHeight();
}

bool TodoItem::isChecked() const
{
    return m_checkBox->isChecked();
}

void TodoItem::setChecked(bool checked)
{
    m_checkBox->setChecked(checked);
}

void TodoItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void TodoItem::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(text());
    mimeData->setData("application/x-todoitem", QByteArray::number(reinterpret_cast<quintptr>(this)));
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}
