#ifndef TODOITEM_H
#define TODOITEM_H

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QEnterEvent>
#include <QDrag>
#include <QMimeData>
#include <QResizeEvent>

class TodoItem : public QWidget
{
    Q_OBJECT

public:
    explicit TodoItem(const QString &text, QWidget *parent = nullptr);

    QString text() const;
    void setText(const QString &text);

    bool isChecked() const;
    void setChecked(bool checked);

signals:
    void deleteRequested();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateHeight();

    QCheckBox *m_checkBox;
    QLabel *m_label;
    QPushButton *m_deleteButton;
    QPoint m_dragStartPosition;
};

#endif
