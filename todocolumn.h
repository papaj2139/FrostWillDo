#ifndef TODOCOLUMN_H
#define TODOCOLUMN_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollArea>
#include <QMouseEvent>
#include <QDrag>
#include "todoitem.h"

class TodoColumn : public QWidget
{
    Q_OBJECT

public:
    explicit TodoColumn(const QString &title, QWidget *parent = nullptr);

    QString title() const;
    void setTitle(const QString &title);

    void addTodoItem(const QString &text);
    void addTodoItem(TodoItem *item);
    void insertTodoItem(int index, TodoItem *item);
    void removeTodoItem(TodoItem *item);

    QList<TodoItem*> todoItems() const;

signals:
    void deleteRequested();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onAddTodo();

private:
    int getDropIndex(const QPoint &pos);

    QLabel *m_titleLabel;
    QPushButton *m_addButton;
    QPushButton *m_deleteButton;
    QVBoxLayout *m_itemsLayout;
    QWidget *m_itemsWidget;
    QScrollArea *m_scrollArea;
    QPoint m_dragStartPosition;
};

#endif
