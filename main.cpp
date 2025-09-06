#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("FrostWillDo");
    app.setApplicationVersion("1.0");

    //set dark theme globally
    app.setStyleSheet(
        "QCheckBox { color: #ffffff; }"
        "QCheckBox::indicator:unchecked { background-color: #2b2b2b; border: 1px solid #606060; }"
        "QCheckBox::indicator:checked { background-color: #4dabf7; border: 1px solid #4dabf7; }"
        "QInputDialog { background-color: #2b2b2b; color: #ffffff; }"
        "QLineEdit { background-color: #1e1e1e; color: #ffffff; border: 1px solid #404040; padding: 4px; }"
        "QScrollBar:vertical { background-color: #2b2b2b; width: 12px; }"
        "QScrollBar::handle:vertical { background-color: #606060; border-radius: 6px; }"
        "QScrollBar::handle:vertical:hover { background-color: #707070; }"
    );

    MainWindow window;
    window.show();

    return app.exec();
}
