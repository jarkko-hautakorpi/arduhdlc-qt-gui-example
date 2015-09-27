#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    a.setApplicationName("qt_arduino_hdlc_example");
    a.setApplicationDisplayName("Qt to Arduino HDLC command router example");
    a.setApplicationVersion("0.1");
    a.setOrganizationName("Jarkko Hautakorpi");
    a.setOrganizationDomain("https://plus.google.com/u/0/+JarkkoHautakorpi/");
    w.show();
    return a.exec();
}
