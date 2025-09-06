QT += widgets

CONFIG += c++17

TARGET = frostwilldo
TEMPLATE = app

SOURCES += main.cpp \
           mainwindow.cpp \
           todoitem.cpp \
           todocolumn.cpp

HEADERS += mainwindow.h \
           todoitem.h \
           todocolumn.h
