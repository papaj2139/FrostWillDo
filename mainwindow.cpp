#include "mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("FrostWillDo");
    setMinimumSize(800, 600);
    setStyleSheet("QMainWindow { background-color: #121212; } QMenuBar { background-color: #1e1e1e; color: #ffffff; } QMenuBar::item:selected { background-color: #404040; }");
    setAcceptDrops(true);

    //set up data file path location: $HOME/.local/share/FrostWillDo/frostwilldo.json
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    m_dataFile = appDataPath + "/frostwilldo.json";

    //menu bar
    QMenuBar *menuBar = this->menuBar();
    QMenu *fileMenu = menuBar->addMenu("&File");

    QAction *addColumnAction = fileMenu->addAction("&Add Column");
    addColumnAction->setShortcut(QKeySequence::New);
    connect(addColumnAction, &QAction::triggered, this, &MainWindow::addColumn);

    fileMenu->addSeparator();

    QAction *saveAction = fileMenu->addAction("&Save");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveData);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    //central widget setup
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_centralWidget = new QWidget();
    m_centralWidget->setStyleSheet("background-color: #121212;");

    m_columnsLayout = new QHBoxLayout(m_centralWidget);
    m_columnsLayout->setContentsMargins(16, 16, 16, 16);
    m_columnsLayout->setSpacing(16);
    m_columnsLayout->addStretch();

    m_scrollArea->setWidget(m_centralWidget);
    setCentralWidget(m_scrollArea);

    //load saved data or create defaults
    loadData();

    //setup auto-save
    setupAutoSave();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-todocolumn")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-todocolumn")) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-todocolumn")) {
        QByteArray columnData = event->mimeData()->data("application/x-todocolumn");
        TodoColumn *column = reinterpret_cast<TodoColumn*>(columnData.toLongLong());

        if (column) {
            //remove from current position
            m_columnsLayout->removeWidget(column);

            //insert at new position
            QPoint centralPos = m_centralWidget->mapFrom(this, event->position().toPoint());
            int dropIndex = getColumnDropIndex(centralPos);
            m_columnsLayout->insertWidget(dropIndex, column);

            autoSave(); //save the new order
        }

        event->acceptProposedAction();
    }
}

int MainWindow::getColumnDropIndex(const QPoint &pos)
{
    for (int i = 0; i < m_columnsLayout->count() - 1; ++i) { //-1 to skip stretch
        QWidget *widget = m_columnsLayout->itemAt(i)->widget();
        if (widget && pos.x() < widget->x() + widget->width() / 2) {
            return i;
        }
    }
    return m_columnsLayout->count() - 1; //insert before stretch
}

void MainWindow::addColumn()
{
    bool ok;
    QString title = QInputDialog::getText(this, "Add Column", "Column title:", QLineEdit::Normal, "", &ok);
    if (ok && !title.isEmpty()) {
        TodoColumn *column = new TodoColumn(title, this);
        m_columnsLayout->insertWidget(m_columnsLayout->count() - 1, column);
        connect(column, &TodoColumn::deleteRequested, this, &MainWindow::deleteColumn);
        autoSave(); //save immediately when adding column
    }
}

void MainWindow::deleteColumn()
{
    TodoColumn *column = qobject_cast<TodoColumn*>(sender());
    if (column) {
        int ret = QMessageBox::question(this, "Delete Column",
                                       QString("Are you sure you want to delete the column '%1'?")
                                       .arg(column->title()),
                                       QMessageBox::Yes | QMessageBox::No);

        if (ret == QMessageBox::Yes) {
            m_columnsLayout->removeWidget(column);
            column->deleteLater();
            autoSave(); //save immediately when deleting column
        }
    }
}

void MainWindow::saveData()
{
    QJsonObject root;
    QJsonArray columnsArray;

    for (TodoColumn *column : columns()) {
        QJsonObject columnObj;
        columnObj["title"] = column->title();

        QJsonArray todosArray;
        for (TodoItem *item : column->todoItems()) {
            QJsonObject todoObj;
            todoObj["text"] = item->text();
            todoObj["checked"] = item->isChecked();
            todosArray.append(todoObj);
        }
        columnObj["todos"] = todosArray;
        columnsArray.append(columnObj);
    }

    root["columns"] = columnsArray;

    QJsonDocument doc(root);
    QFile file(m_dataFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void MainWindow::loadData()
{
    QFile file(m_dataFile);
    if (!file.open(QIODevice::ReadOnly)) {
        //create default columns if no save file exists
        TodoColumn *todoColumn = new TodoColumn("To Do", this);
        TodoColumn *doingColumn = new TodoColumn("Doing", this);
        TodoColumn *doneColumn = new TodoColumn("Done", this);

        m_columnsLayout->insertWidget(m_columnsLayout->count() - 1, todoColumn);
        m_columnsLayout->insertWidget(m_columnsLayout->count() - 1, doingColumn);
        m_columnsLayout->insertWidget(m_columnsLayout->count() - 1, doneColumn);

        connect(todoColumn, &TodoColumn::deleteRequested, this, &MainWindow::deleteColumn);
        connect(doingColumn, &TodoColumn::deleteRequested, this, &MainWindow::deleteColumn);
        connect(doneColumn, &TodoColumn::deleteRequested, this, &MainWindow::deleteColumn);

        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();
    QJsonArray columnsArray = root["columns"].toArray();

    for (const QJsonValue &columnValue : columnsArray) {
        QJsonObject columnObj = columnValue.toObject();
        QString title = columnObj["title"].toString();

        TodoColumn *column = new TodoColumn(title, this);
        m_columnsLayout->insertWidget(m_columnsLayout->count() - 1, column);
        connect(column, &TodoColumn::deleteRequested, this, &MainWindow::deleteColumn);

        QJsonArray todosArray = columnObj["todos"].toArray();
        for (const QJsonValue &todoValue : todosArray) {
            QJsonObject todoObj = todoValue.toObject();
            QString text = todoObj["text"].toString();
            bool checked = todoObj["checked"].toBool();

            column->addTodoItem(text);
            //set checked state after adding
            QList<TodoItem*> items = column->todoItems();
            if (!items.isEmpty()) {
                items.last()->setChecked(checked);
            }
        }
    }
}

void MainWindow::autoSave()
{
    saveData();
}

void MainWindow::setupAutoSave()
{
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setSingleShot(false);
    m_autoSaveTimer->setInterval(30000); //auto save every 30 seconds
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    m_autoSaveTimer->start();
}

QList<TodoColumn*> MainWindow::columns() const
{
    QList<TodoColumn*> columnList;
    for (int i = 0; i < m_columnsLayout->count() - 1; ++i) { //-1 to skip the stretch
        if (TodoColumn *column = qobject_cast<TodoColumn*>(m_columnsLayout->itemAt(i)->widget())) {
            columnList.append(column);
        }
    }
    return columnList;
}
