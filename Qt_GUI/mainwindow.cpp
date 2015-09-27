#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hdlc_qt.h"
#include "defined_commands.h"
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->showMessage("Qt to Arduino HDLC command router example", 3000);
    serial = new QSerialPort(this);
    this->fillPortsInfo();
    HDLC_qt* hdlc = HDLC_qt::instance();

    QObject::connect(hdlc, SIGNAL(hdlcTransmitByte(QByteArray)),
                     this, SLOT(putChar(QByteArray)));

    QObject::connect(hdlc, SIGNAL(hdlcTransmitByte(char)),
                     this, SLOT(putChar(char)));

    QObject::connect(hdlc, SIGNAL(hdlcValidFrameReceived(QByteArray, quint16)),
                     this, SLOT(HDLC_CommandRouter(QByteArray, quint16)));

    // All data coming from serial port (should go straight to HDLC validation)
    QObject::connect(this, SIGNAL(dataReceived(QByteArray)),
                     hdlc, SLOT(charReceiver(QByteArray)));

    // Allso echo chars HEX value
    QObject::connect(this, SIGNAL(dataReceived(QByteArray)),
                     this, SLOT(print_hex_value(QByteArray)));

    QObject::connect(this, SIGNAL(sendDataFrame(QByteArray, quint16)),
                     hdlc, SLOT(frameDecode(QByteArray, quint16)));

    QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                     this, SLOT(openSerialPort()));

    QObject::connect(serial, SIGNAL(readyRead()),
                     this, SLOT(readData()));

    QObject::connect(ui->dial, SIGNAL(valueChanged(int)),
                     this, SLOT(setServoPosition()) );

    QObject::connect(ui->pushButton_echo, SIGNAL(clicked()),
                     this, SLOT(sendEcho()) );

    QObject::connect(ui->pushButton_LED, SIGNAL(clicked()),
                     this, SLOT(toggleLED()) );
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::putChar(QByteArray data)
{
    ui->plainTextEdit_debug->appendPlainText(data.data());
    this->writeData(data);
}

void MainWindow::putChar(char data)
{
    QByteArray qdata(1, data);
    ui->plainTextEdit_debug->appendPlainText(qdata.data());
    this->writeData(qdata);
}

void MainWindow::HDLC_CommandRouter(QByteArray buffer, quint16 bytes_received)
{
    enum serial_responses command = static_cast<serial_responses>(buffer.at(0));
    switch(command)
    {
        case RESPONSE_ERROR:            this->command_error(); break;
        case RESPONSE_VERSION:          this->command_default(buffer, bytes_received); break;
        case RESPONSE_BUTTON_PRESS:  	this->command_default(buffer, bytes_received); break;
        case RESPONSE_READ_SIGNATURE:   this->response_read_signature(buffer, bytes_received); break;
        case RESPONSE_ECHO_DATA:        this->command_default(buffer, bytes_received); break;
        default:
            this->command_error();
            break;
    }
}

void MainWindow::command_default(QByteArray buffer, quint16 bytes_received) {
    ui->statusBar->showMessage(tr("Received a valid HDLC packet/command"), 2000);
}

void MainWindow::command_error() {
    ui->statusBar->showMessage(tr("Error in HDLC packet/command"), 2000);
}

void MainWindow::response_read_signature(QByteArray buffer, quint16 bytes_received) {
    /* Device signature bytes */
    QString device_signature = "0x";
    QByteArray signature_bytes;
    signature_bytes
            .append(buffer.at(1))
            .append(buffer.at(2))
            .append(buffer.at(3));
    device_signature.append(signature_bytes.toHex().toUpper());
    ui->label_deviceSignature->setText(device_signature);

    /* Oscillator calibration byte */
    QString oscillator_calibr = "0x";
    QByteArray oscillator_byte;
    oscillator_byte.append(buffer.at(4));
    oscillator_calibr.append(oscillator_byte.toHex().toUpper());
    ui->label_oscillatorCalibrationByte->setText(oscillator_calibr);
}

void MainWindow::response_echo_data(QByteArray buffer, quint16 bytes_received) {
    QString bufferText(buffer);
    ui->statusBar->showMessage(bufferText, 2000);
}


void MainWindow::sendData() {
    QByteArray data;
    data.append((quint8) ui->lineEdit_Command->text().toInt());
    data.append(ui->lineEdit_Data->text());;
    emit sendDataFrame(data, (quint16)data.length());
}

void MainWindow::fillPortsInfo()
{
    ui->serialPortDropdown->clear();
    static const QString blankString = QObject::tr("N/A");
    QString description;
    QString manufacturer;
    QString serialNumber;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortDropdown->addItem(list.first(), list);
    }
}

void MainWindow::openSerialPort()
{
    serial->setPortName(ui->serialPortDropdown->currentText());
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)) {
            ui->statusBar->showMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                                       .arg(ui->serialPortDropdown->currentText())
                                       .arg(QSerialPort::Baud9600)
                                       .arg(QSerialPort::Data8)
                                       .arg(QSerialPort::NoParity)
                                       .arg(QSerialPort::OneStop)
                                       .arg(QSerialPort::NoFlowControl));
            ui->pushButton_connect->setText("Disconnect");
            disconnect( ui->pushButton_connect, SIGNAL(clicked()),0, 0);
            QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                             this, SLOT(closeSerialPort()));

    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());
        ui->statusBar->showMessage(tr("Open error"));
    }
    ui->plainTextEdit_input->appendPlainText("Getting chip signature:\n");
    QByteArray data;
    data.append(COMMAND_READ_SIGNATURE);
    emit sendDataFrame(data, 1);
}

void MainWindow::closeSerialPort()
{
    serial->close();
    ui->statusBar->showMessage(tr("Disconnected"));
    disconnect( ui->pushButton_connect, SIGNAL(clicked()),0, 0);
    QObject::connect(ui->pushButton_connect, SIGNAL(clicked()),
                     this, SLOT(openSerialPort()));
    ui->pushButton_connect->setText("Connect");
    ui->label_deviceSignature->setText(" ");
    ui->label_oscillatorCalibrationByte->setText(" ");
}

void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}

void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    ui->plainTextEdit_input->appendPlainText(data);
    int dSize = data.size();
    if (dSize > 0) {
        emit dataReceived(data);
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}

void MainWindow::sendData(QByteArray data) {
    quint16 length = data.length();
    emit sendDataFrame(data, length);
}

void MainWindow::setStatusbarText(const QString& text) {
    ui->statusBar->showMessage(text);
}

void MainWindow::print_hex_value(QByteArray data) {
    QByteArray hex_console;
    QByteArray character;
    for (int i = 0; i < data.size(); ++i) {
        character.clear();
        character.append(data.at(i));
        //character = data.at(i);
        hex_console.append("0x");
        hex_console.append(character.toHex().toUpper());
        hex_console.append("\n");
    }
    ui->plainTextEdit_input_hex->appendPlainText(QString(hex_console));
}

/* Command sending functions */

void MainWindow::setServoPosition() {
    QByteArray data;
    quint16 length;
    quint16 position = ui->dial->value();
    data.append(COMMAND_SET_SERVO_POSITION);
    //data.append(position >> 8); // high byte
    data.append(position & 0xff); // low byte
    length = (quint16) data.length();
    emit sendDataFrame(data, length);
}

void MainWindow::sendEcho() {
    QByteArray data;
    data.append((quint8)COMMAND_ECHO_DATA);
    data.append("ABCD");
    emit sendDataFrame(data, (quint16) data.length());
}

void MainWindow::toggleLED() {
    QByteArray data;
    data.append((quint8)COMMAND_TOGGLE_LED);
    emit sendDataFrame(data, 1);
}
