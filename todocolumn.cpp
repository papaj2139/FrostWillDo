#include "todocolumn.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QDrag>

TodoColumn::TodoColumn(const QString &title, QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
    setFixedWidth(300); //slightly wider for better text display
    setStyleSheet(
        "TodoColumn { "
        "   background-color: #1e1e1e; "
        "   border: 1px solid #404040; "
        "   border-radius: 8px; "
        "}"
    );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    //header with title and buttons
    QHBoxLayout *headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #ffffff; padding: 4px;");
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_deleteButton = new QPushButton("×", this);
    m_deleteButton->setFixedSize(20, 20);
    m_deleteButton->setStyleSheet("QPushButton { color: #ff6b6b; font-weight: bold; border: none; background-color: transparent; } QPushButton:hover { background-color: #3d1a1a; border-radius: 10px; }");

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_deleteButton);

    mainLayout->addLayout(headerLayout);

    //add todo button
    m_addButton = new QPushButton("+ Add Todo", this);
    m_addButton->setStyleSheet(
        "QPushButton { "
        "   background-color: #4dabf7; "
        "   color: #ffffff; "
        "   border: none; "
        "   padding: 8px; "
        "   border-radius: 4px; "
        "} "
        "QPushButton:hover { "
        "   background-color: #339af0; "
        "}"
    );
    mainLayout->addWidget(m_addButton);

    //scroll area for todo items
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background-color: transparent; }");

    m_itemsWidget = new QWidget();
    m_itemsLayout = new QVBoxLayout(m_itemsWidget);
    m_itemsLayout->setContentsMargins(0, 0, 0, 0);
    m_itemsLayout->setSpacing(4);
    m_itemsLayout->addStretch();

    m_scrollArea->setWidget(m_itemsWidget);
    mainLayout->addWidget(m_scrollArea, 1);

    connect(m_addButton, &QPushButton::clicked, this, &TodoColumn::onAddTodo);
    connect(m_deleteButton, &QPushButton::clicked, this, &TodoColumn::deleteRequested);
}

QString TodoColumn::title() const
{
    return m_titleLabel->text();
}

void TodoColumn::setTitle(const QString &title)
{
    m_titleLabel->setText(title);
}

void TodoColumn::addTodoItem(const QString &text)
{
    TodoItem *item = new TodoItem(text, this);
    addTodoItem(item);
}

void TodoColumn::addTodoItem(TodoItem *item)
{
    item->setParent(m_itemsWidget);
    m_itemsLayout->insertWidget(m_itemsLayout->count() - 1, item);

    //connect delete signal
    connect(item, &TodoItem::deleteRequested, this, [this, item]() {
        removeTodoItem(item);
        item->deleteLater();
    });
}

void TodoColumn::insertTodoItem(int index, TodoItem *item)
{
    item->setParent(m_itemsWidget);
    m_itemsLayout->insertWidget(qMin(index, m_itemsLayout->count() - 1), item);

    //connect delete signal
    connect(item, &TodoItem::deleteRequested, this, [this, item]() {
        removeTodoItem(item);
        item->deleteLater();
    });
}

void TodoColumn::removeTodoItem(TodoItem *item)
{
    m_itemsLayout->removeWidget(item);
    item->setParent(nullptr);
}

QList<TodoItem*> TodoColumn::todoItems() const
{
    QList<TodoItem*> items;
    for (int i = 0; i < m_itemsLayout->count() - 1; ++i) { //-1 to skip the stretch
        if (TodoItem *item = qobject_cast<TodoItem*>(m_itemsLayout->itemAt(i)->widget())) {
            items.append(item);
        }
    }
    return items;
}

void TodoColumn::onAddTodo()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Add Todo", "Todo text:", QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty()) {
        addTodoItem(text);
    }
}

int TodoColumn::getDropIndex(const QPoint &pos)
{
    //find the best insertion point based on Y position
    QPoint scrollAreaPos = m_scrollArea->mapFrom(this, pos);
    QPoint itemsPos = m_itemsWidget->mapFrom(m_scrollArea, scrollAreaPos);

    for (int i = 0; i < m_itemsLayout->count() - 1; ++i) {
        QWidget *widget = m_itemsLayout->itemAt(i)->widget();
        if (widget && itemsPos.y() < widget->y() + widget->height() / 2) {
            return i;
        }
    }
    return m_itemsLayout->count() - 1; //insert before stretch
}

void TodoColumn::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-todoitem")) {
        event->acceptProposedAction();
    }
}

void TodoColumn::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-todoitem")) {
        event->acceptProposedAction();
    }
}

void TodoColumn::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-todoitem")) {
        QByteArray itemData = event->mimeData()->data("application/x-todoitem");
        TodoItem *item = reinterpret_cast<TodoItem*>(itemData.toLongLong());

        if (item) {
            //remove from old parent
            if (TodoColumn *oldColumn = qobject_cast<TodoColumn*>(item->parent()->parent()->parent())) {
                oldColumn->removeTodoItem(item);
            }

            //insert at the correct position in this column
            int dropIndex = getDropIndex(event->position().toPoint());
            insertTodoItem(dropIndex, item);
        }

        event->acceptProposedAction();
    }
}

void TodoColumn::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->position().y() < 50) { //only drag from header area
        m_dragStartPosition = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void TodoColumn::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if (event->position().y() > 50) //dont drag if not in header area
        return;
    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setText(title());
    mimeData->setData("application/x-todocolumn", QByteArray::number(reinterpret_cast<quintptr>(this)));
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}
