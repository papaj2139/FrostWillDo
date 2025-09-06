#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QMenuBar>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include "todocolumn.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void addColumn();
    void deleteColumn();
    void saveData();
    void loadData();
    void autoSave();

private:
    void setupAutoSave();
    QList<TodoColumn*> columns() const;
    int getColumnDropIndex(const QPoint &pos);

    QScrollArea *m_scrollArea;
    QWidget *m_centralWidget;
    QHBoxLayout *m_columnsLayout;
    QTimer *m_autoSaveTimer;
    QString m_dataFile;
};

#endif
