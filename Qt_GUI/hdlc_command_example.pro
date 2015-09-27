
QT       += core gui serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_ardu_hdlc_example
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        hdlc_qt.cpp

HEADERS  += mainwindow.h \
    hdlc_qt.h \
    defined_commands.h

FORMS    += mainwindow.ui

