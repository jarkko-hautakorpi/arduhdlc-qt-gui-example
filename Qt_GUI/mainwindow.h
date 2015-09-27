#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QtGlobal>
#include <QMainWindow>
#include <QObject>
#include <QtSerialPort/QtSerialPort>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void putChar(QByteArray data);
    void putChar(char data);
    void HDLC_CommandRouter(QByteArray buffer, quint16 bytes_received);
    void setStatusbarText(const QString& text);
    void setServoPosition();
    void sendEcho();
    void toggleLED();

signals:
    void sendDataFrame(QByteArray buffer, quint16 size);
    void dataReceived(QByteArray data);

private:
    Ui::MainWindow *ui;
    void fillPortsInfo();
    QSerialPort *serial;
    void getChipSignature();
    void command_default(QByteArray buffer, quint16 bytes_received);
    void response_read_signature(QByteArray buffer, quint16 bytes_received);
    void response_echo_data(QByteArray buffer, quint16 bytes_received);
    void command_error();


private slots:
    void sendData();
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void sendData(QByteArray data);
    void handleError(QSerialPort::SerialPortError error);
    void print_hex_value(QByteArray data);
};

#endif // MAINWINDOW_H
