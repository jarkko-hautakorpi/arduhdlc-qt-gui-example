#ifndef HDLC_qt_H
#define HDLC_qt_H

#include <QObject>
#include <QtCore/QtGlobal>
#include <QString>
#include <QMutex>
#include <QQueue>

/* HDLC Asynchronous framing */
/* The frame boundary octet is 01111110, (7E in hexadecimal notation) */
#define FRAME_BOUNDARY_OCTET 0x7E

/* A "control escape octet", has the bit sequence '01111101', (7D hexadecimal) */
#define CONTROL_ESCAPE_OCTET 0x7D

/* If either of these two octets appears in the transmitted data, an escape octet is sent, */
/* followed by the original data octet with bit 5 inverted */
#define INVERT_OCTET 0x20

/* 16bit low and high bytes copier */
#define low(x)    ((x) & 0xFF)
#define high(x)   (((x)>>8) & 0xFF)


class HDLC_qt : public QObject
{
    Q_OBJECT

public:
    static HDLC_qt* instance()
    {
        static QMutex mutex;
        if (!m_Instance)
        {
            mutex.lock();
            if (!m_Instance)
                m_Instance = new HDLC_qt;
            mutex.unlock();
        }
        return m_Instance;
    }

    static void drop()
    {
        static QMutex mutex;
        mutex.lock();
        delete m_Instance;
        m_Instance = 0;
        mutex.unlock();
    }

signals:
    /* Signals can never have return types (i.e. use void) */
    void hdlcTransmitByte(QByteArray data);
    void hdlcTransmitByte(char data);
    void hdlcValidFrameReceived(QByteArray receive_frame_buffer, quint16 frame_size);

public slots:
    void charReceiver(QByteArray dataArray);
    void frameDecode(QByteArray buffer, quint16 bytes_to_send);

private:
    HDLC_qt(){}
    HDLC_qt(const HDLC_qt &); // hide copy constructor
    HDLC_qt& operator=(const HDLC_qt &);// hide assign op
                                        // we leave just the declarations, so the compiler will warn us
                                        // if we try to use those two functions by accident

    static HDLC_qt* m_Instance;
    QByteArray receive_frame_buffer;
    quint16 frame_position;
    quint16 frame_checksum;
    bool escape_character;
    bool hdlcFrameCRC_check(int frame_index);
};

#endif // HDLC_qt_H
